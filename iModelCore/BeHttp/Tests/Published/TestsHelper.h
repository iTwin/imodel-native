/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/TestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/BeTest.h>
#include <BeHttp/Http.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

#define BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE BEGIN_BENTLEY_HTTP_NAMESPACE namespace UnitTests {
#define END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE   } END_BENTLEY_HTTP_NAMESPACE
#define USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS using namespace BentleyApi::Http::UnitTests;

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_HTTP

BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE

Json::Value ToJson (Utf8StringCR jsonString);
std::shared_ptr<rapidjson::Document> ToRapidJson (Utf8StringCR jsonString);

template<typename T>
bvector<T> StubBVector (T element)
    {
    bvector<T> vector;
    vector.push_back (element);
    return vector;
    }

template<typename T>
bvector<T> StubBVector (std::initializer_list<T> list)
    {
    bvector<T> vector (list.begin (), list.end ());
    return vector;
    }

template<typename T>
bset<T> StubBSet (std::initializer_list<T> list)
    {
    bset<T> set (list.begin (), list.end ());
    return set;
    }

END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE