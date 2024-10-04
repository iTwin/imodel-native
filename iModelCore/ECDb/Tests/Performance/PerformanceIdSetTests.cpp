/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct PerformanceIdSetTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceIdSetTests, SimpleComparisonBetweenInVirtualSet_and_IdSet)
    {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SimpleComparisonBetweenInVirtualSet_and_IdSet.ecdb"));
        ECSqlStatement stmt_With_InvirtualSet;
        ECSqlStatement stmt_With_IdSet;
        std::vector<Utf8String> v = {"0x373","0x38F", "0x32", "0x01"};
        IdSet<BeInt64Id> emptyIdset;
        for(auto str: v)
        {
            emptyIdset.insert(BeInt64Id(BeStringUtilities::ParseHex(str.c_str())));
        } 
        std::shared_ptr<IdSet<BeInt64Id>> idSetPtr = std::make_shared<IdSet<BeInt64Id>>(emptyIdset);
        ASSERT_EQ(ECSqlStatus::Success, stmt_With_InvirtualSet.Prepare(m_ecdb, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt where invirtualset(?, x)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt_With_InvirtualSet.BindVirtualSet(1, idSetPtr));

        ASSERT_EQ(ECSqlStatus::Success, stmt_With_IdSet.Prepare(m_ecdb, "select x from ECVLib.IdSet(?), (with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt) where id = x"));
        IECSqlBinder& binder = stmt_With_IdSet.GetBinder(1);
        for(auto str: v)
        {
            ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindText(str.c_str(), IECSqlBinder::MakeCopy::No));
        } 

        auto iterator_set = emptyIdset.begin();
        int i = 0;
        StopWatch timer_InVirtual_Set(true);
        while(stmt_With_InvirtualSet.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((*iterator_set).GetValue(), stmt_With_InvirtualSet.GetValueInt64(0));
            ++iterator_set;
            i++;
        }
        timer_InVirtual_Set.Stop();
        LOGTODB(TEST_DETAILS, timer_InVirtual_Set.GetElapsedSeconds(), i);

        iterator_set = emptyIdset.begin();
        i = 0;
        StopWatch timer_IdSet(true);
        while(stmt_With_InvirtualSet.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((*iterator_set).GetValue(), stmt_With_InvirtualSet.GetValueInt64(0));
            ++iterator_set;
            i++;
        }
        timer_IdSet.Stop();
        LOGTODB(TEST_DETAILS, timer_IdSet.GetElapsed(), i);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceIdSetTests, SimpleComparisonBetweenInVirtualSet_and_IdSet_with_large_set)
    {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SimpleComparisonBetweenInVirtualSet_and_IdSet_with_large_set.ecdb"));
        ECSqlStatement stmt_With_InvirtualSet;
        ECSqlStatement stmt_With_IdSet;
        IdSet<BeInt64Id> emptyIdset;

        ASSERT_EQ(ECSqlStatus::Success, stmt_With_InvirtualSet.Prepare(m_ecdb, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt where invirtualset(?, x)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt_With_IdSet.Prepare(m_ecdb, "select x from ECVLib.IdSet(?), (with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt) where id = x"));
        
        IECSqlBinder& binder = stmt_With_IdSet.GetBinder(1);
        for(int i = 0;i<2000;i++)
        {
            int randNum = std::rand()%(50000-1 + 1) + 1;
            ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindInt64(randNum));
            emptyIdset.insert(BeInt64Id(randNum));
        } 
        std::shared_ptr<IdSet<BeInt64Id>> idSetPtr = std::make_shared<IdSet<BeInt64Id>>(emptyIdset);
        ASSERT_EQ(ECSqlStatus::Success, stmt_With_InvirtualSet.BindVirtualSet(1, idSetPtr));


        auto iterator_set = emptyIdset.begin();
        int i = 0;
        StopWatch timer_InVirtual_Set(true);
        while(stmt_With_InvirtualSet.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((*iterator_set).GetValue(), stmt_With_InvirtualSet.GetValueInt64(0));
            ++iterator_set;
            i++;
        }
        timer_InVirtual_Set.Stop();
        LOGTODB(TEST_DETAILS, timer_InVirtual_Set.GetElapsedSeconds(), i);

        iterator_set = emptyIdset.begin();
        i = 0;
        StopWatch timer_IdSet(true);
        while(stmt_With_InvirtualSet.Step() == BE_SQLITE_ROW)
        {
            ASSERT_EQ((*iterator_set).GetValue(), stmt_With_InvirtualSet.GetValueInt64(0));
            ++iterator_set;
            i++;
        }
        timer_IdSet.Stop();
        LOGTODB(TEST_DETAILS, timer_IdSet.GetElapsed(), i);
    }
END_ECDBUNITTESTS_NAMESPACE