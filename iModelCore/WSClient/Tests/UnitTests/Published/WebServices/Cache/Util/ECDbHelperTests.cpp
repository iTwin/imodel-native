/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ECDbHelperTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECDbHelperTests.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(ECDbHelperTests, Erase_MultipleInstanceKeysInMap_ErasesSpecificKeyFromMap)
    {
    ECInstanceKey key1(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(2)));
    ECInstanceKey key2(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(3)));

    ECInstanceKeyMultiMap map;
    map.insert(ECDbHelper::ToPair(key1));
    map.insert(ECDbHelper::ToPair(key2));
    ASSERT_EQ(2, map.size());

    ECDbHelper::Erase(map, key1);

    ASSERT_EQ(1, map.size());
    ASSERT_EQ(ECInstanceId(UINT64_C(3)), map.begin()->second);
    }

TEST_F(ECDbHelperTests, ToECInstanceIdList_MultipleInstanceKeysInMap_CombinesToCommaSeperatedList)
    {
    ECInstanceKeyMultiMap map;
    map.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(4))});
    map.insert({ECClassId(UINT64_C(2)), ECInstanceId(UINT64_C(5))});
    map.insert({ECClassId(UINT64_C(2)), ECInstanceId(UINT64_C(6))});

    EXPECT_STREQ("4,5,6", ECDbHelper::ToECInstanceIdList(map.begin(), map.end()).c_str());
    }

TEST_F(ECDbHelperTests, ToECInstanceIdList_EmptyMap_EmptyString)
    {
    ECInstanceKeyMultiMap map;
    EXPECT_STREQ("", ECDbHelper::ToECInstanceIdList(map.begin(), map.end()).c_str());
    }

TEST_F(ECDbHelperTests, MergeMultiMaps_BothMapsEmpty_ReturnsEmpty)
    {
    ECInstanceKeyMultiMap map1;
    ECInstanceKeyMultiMap map2;

    auto mergedMap = ECDbHelper::MergeMultiMaps(map1, map2);

    ASSERT_EQ(0, mergedMap.size());
    }

TEST_F(ECDbHelperTests, MergeMultiMaps_FirstMapEmpty_ReturnsSecondMap)
    {
    ECInstanceKeyMultiMap map1;
    ECInstanceKeyMultiMap map2;
    map2.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))});

    auto mergedMap = ECDbHelper::MergeMultiMaps(map1, map2);

    ASSERT_EQ(1, mergedMap.size());
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))));
    }

TEST_F(ECDbHelperTests, MergeMultiMaps_SecondMapEmpty_ReturnsFirstMap)
    {
    ECInstanceKeyMultiMap map1;
    map1.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))});

    ECInstanceKeyMultiMap map2;

    auto mergedMap = ECDbHelper::MergeMultiMaps(map1, map2);

    ASSERT_EQ(1, mergedMap.size());
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))));
    }

TEST_F(ECDbHelperTests, MergeMultiMaps_MultipleValues_ReturnsMerged)
    {
    ECInstanceKeyMultiMap map1;
    map1.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))});
    map1.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(2))});
    map1.insert({ECClassId(UINT64_C(2)), ECInstanceId(UINT64_C(1))});

    ECInstanceKeyMultiMap map2;
    map2.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))});
    map2.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(3))});
    map2.insert({ECClassId(UINT64_C(3)), ECInstanceId(UINT64_C(1))});

    auto mergedMap = ECDbHelper::MergeMultiMaps(map1, map2);

    ASSERT_EQ(5, mergedMap.size());
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(2))));
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(3))));
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(2)), ECInstanceId(UINT64_C(1))));
    EXPECT_CONTAINS(mergedMap, ECInstanceKeyMultiMapPair(ECClassId(UINT64_C(3)), ECInstanceId(UINT64_C(1))));
    }