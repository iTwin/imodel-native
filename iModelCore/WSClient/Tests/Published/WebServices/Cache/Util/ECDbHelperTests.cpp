/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/ECDbHelperTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECDbHelperTests.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(ECDbHelperTests, Erase_MultipleInstanceKeysInMap_ErasesSpecificKeyFromMap)
    {
    ECInstanceKey key1(1, ECInstanceId(2));
    ECInstanceKey key2(1, ECInstanceId(3));

    ECInstanceKeyMultiMap map;
    map.insert(ECDbHelper::ToPair(key1));
    map.insert(ECDbHelper::ToPair(key2));
    ASSERT_EQ(2, map.size());

    ECDbHelper::Erase(map, key1);

    ASSERT_EQ(1, map.size());
    ASSERT_EQ(ECInstanceId(3), map.begin()->second);
    }

TEST_F(ECDbHelperTests, ToECInstanceIdList_MultipleInstanceKeysInMap_CombinesToCommaSeperatedList)
    {
    ECInstanceKeyMultiMap map;
    map.insert({1, ECInstanceId(4)});
    map.insert({2, ECInstanceId(5)});
    map.insert({2, ECInstanceId(6)});

    EXPECT_STREQ("4,5,6", ECDbHelper::ToECInstanceIdList(map.begin(), map.end()).c_str());
    }

TEST_F(ECDbHelperTests, ToECInstanceIdList_EmptyMap_EmptyString)
    {
    ECInstanceKeyMultiMap map;
    EXPECT_STREQ("", ECDbHelper::ToECInstanceIdList(map.begin(), map.end()).c_str());
    }
