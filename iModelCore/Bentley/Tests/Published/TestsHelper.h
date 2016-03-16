/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/TestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <rapidjson/BeRapidJson.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/BeTest.h>
#include <BeHttp/Http.h>


#define BEGIN_BENTLEY_UNIT_TESTS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace UnitTests {
#define END_BENTLEY_UNIT_TESTS_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_UNIT_TESTS using namespace BentleyApi::UnitTests;

USING_NAMESPACE_BENTLEY
#define EXPECT_CONTAINS(container, value)                                       \
    EXPECT_FALSE (std::find (container.begin (), container.end (), value) == container.end ())

#define EXPECT_NCONTAIN(container, value)                                       \
    EXPECT_TRUE (std::find (container.begin (), container.end (), value) == container.end ())

#define EXPECT_BETWEEN(smallerValue, value, biggerValue)                        \
    EXPECT_LE (smallerValue, value);                                            \
    EXPECT_GE (biggerValue, value)

BEGIN_BENTLEY_UNIT_TESTS_NAMESPACE

//Json::Value ToJson (Utf8StringCR jsonString);
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

END_BENTLEY_UNIT_TESTS_NAMESPACE