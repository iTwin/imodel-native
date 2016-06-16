/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/TestsHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

Json::Value ToJson (Utf8StringCR jsonString)
    {
    Json::Value json;
    bool success = Json::Reader::Parse (jsonString, json);
    BeAssert (success && "Check json string");
    return json;
    }

std::shared_ptr<rapidjson::Document> ToRapidJson (Utf8StringCR jsonString)
    {
    auto json = std::make_shared<rapidjson::Document>();
    bool fail = json->Parse<rapidjson::kParseDefaultFlags> (jsonString.c_str()).HasParseError();
    BeAssert (!fail && "Check json string");
    return json;
    }
