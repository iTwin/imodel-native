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
        ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
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

        ASSERT_EQ(ECSqlStatus::Success, stmt_With_IdSet.Prepare(m_ecdb, "select x from IdSet(?), (with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt) where id = x"));
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
        LOGTODB(TEST_DETAILS, timer_InVirtual_Set.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_InvirtualSet.GetECSql()));

        iterator_set = emptyIdset.begin();
        i = 0;
        StopWatch timer_IdSet(true);
        while(stmt_With_IdSet.Step() == BE_SQLITE_ROW)
            {
            ASSERT_EQ((*iterator_set).GetValue(), stmt_With_IdSet.GetValueInt64(0));
            ++iterator_set;
            i++;
            }
        timer_IdSet.Stop();
        LOGTODB(TEST_DETAILS, timer_IdSet.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_IdSet.GetECSql()));
        ASSERT_TRUE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_FALSE(EnableECSqlExperimentalFeatures(m_ecdb, false));
        }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceIdSetTests, SimpleComparisonBetweenInVirtualSet_and_IdSet_with_large_set)
        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SimpleComparisonBetweenInVirtualSet_and_IdSet_with_large_set.ecdb"));
        ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
        ECSqlStatement stmt_With_InvirtualSet;
        ECSqlStatement stmt_With_IdSet;
        IdSet<BeInt64Id> emptyIdset;

        ASSERT_EQ(ECSqlStatus::Success, stmt_With_InvirtualSet.Prepare(m_ecdb, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt where invirtualset(?, x)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt_With_IdSet.Prepare(m_ecdb, "select x from IdSet(?), (with cnt(x) as (values(1) union select x+1 from cnt where x < 1000000 ) select * from cnt) where x = id"));
        
        IECSqlBinder& binder = stmt_With_IdSet.GetBinder(1);
        for(int i = 0;i<2000;i++)
            {
            int randNum = i+1;
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
        LOGTODB(TEST_DETAILS, timer_InVirtual_Set.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_InvirtualSet.GetECSql()));

        iterator_set = emptyIdset.begin();
        i = 0;
        StopWatch timer_IdSet(true);
        while(stmt_With_IdSet.Step() == BE_SQLITE_ROW)
            {
            ASSERT_EQ((*iterator_set).GetValue(), stmt_With_IdSet.GetValueInt64(0));
            ++iterator_set;
            i++;
            }
        timer_IdSet.Stop();
        LOGTODB(TEST_DETAILS, timer_IdSet.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_IdSet.GetECSql()));
        ASSERT_TRUE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_FALSE(EnableECSqlExperimentalFeatures(m_ecdb, false));
        }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceIdSetTests, SimpleComparisonBetweenInVirtualSet_and_IdSet_with_custom_schema)
        {
        ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("SimpleComparisonBetweenInVirtualSet_and_IdSet_with_custom_schema.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="UnIndexed_Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

        ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
        ECSqlStatement insert_stmt;
        ASSERT_EQ(ECSqlStatus::Success, insert_stmt.Prepare(m_ecdb, "insert into ts.Foo(UnIndexed_Prop) values(?)"));
        for(int i = 0;i<1000000;i++)
            {
            insert_stmt.ClearBindings();
            insert_stmt.Reset();
            int rand = (i+1) % 70000;
            insert_stmt.BindInt64(1, rand);
            ASSERT_EQ(BE_SQLITE_DONE, insert_stmt.Step());
            }

        std::vector<Utf8String> v = {"ECInstanceId", "UnIndexed_Prop"};
        for(auto& str: v)
            {
            ECSqlStatement stmt_With_InvirtualSet;
            ECSqlStatement stmt_With_IdSet;
            IdSet<BeInt64Id> emptyIdset;

            ASSERT_EQ(ECSqlStatus::Success, stmt_With_InvirtualSet.Prepare(m_ecdb,SqlPrintfString("select %s from ts.Foo where invirtualset(?, %s) group by %s", str.c_str(), str.c_str(), str.c_str())));
            ASSERT_EQ(ECSqlStatus::Success, stmt_With_IdSet.Prepare(m_ecdb,SqlPrintfString("select %s from IdSet(?), ts.Foo where %s = id group by %s", str.c_str(), str.c_str(), str.c_str())));
            
            IECSqlBinder& binder = stmt_With_IdSet.GetBinder(1);
            for(int i = 0;i<2000;i++)
                {
                int randNum = i+1;
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
            LOGTODB(TEST_DETAILS, timer_InVirtual_Set.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_InvirtualSet.GetECSql()));

            iterator_set = emptyIdset.begin();
            i = 0;
            StopWatch timer_IdSet(true);
            while(stmt_With_IdSet.Step() == BE_SQLITE_ROW)
                {
                ASSERT_EQ((*iterator_set).GetValue(), stmt_With_IdSet.GetValueInt64(0));
                ++iterator_set;
                i++;
                }
            timer_IdSet.Stop();
            LOGTODB(TEST_DETAILS, timer_IdSet.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_IdSet.GetECSql()));
            }
        ASSERT_TRUE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_FALSE(EnableECSqlExperimentalFeatures(m_ecdb, false));
        }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceIdSetTests, SimpleComparisonBetweenJoinedQuery_and_QueryWithWhereClause_with_IdSet)
        {
        ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("SimpleComparisonBetweenJoinedQuery_and_QueryWithWhereClause_with_IdSet.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="UnIndexed_Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

        ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

        ECSqlStatement insert_stmt;
        ASSERT_EQ(ECSqlStatus::Success, insert_stmt.Prepare(m_ecdb, "insert into ts.Foo(UnIndexed_Prop) values(?)"));
        for(int i = 0;i<1000000;i++)
            {
            insert_stmt.ClearBindings();
            insert_stmt.Reset();
            int rand = (i+1) % 70000;
            insert_stmt.BindInt64(1, rand);
            ASSERT_EQ(BE_SQLITE_DONE, insert_stmt.Step());
            }

        std::vector<Utf8String> v = {"ECInstanceId", "UnIndexed_Prop"};
        for(auto& str: v)
            {
            ECSqlStatement stmt_With_Join;
            ECSqlStatement stmt_With_WhereClause;
            IdSet<BeInt64Id> emptyIdset;

            ASSERT_EQ(ECSqlStatus::Success, stmt_With_Join.Prepare(m_ecdb,SqlPrintfString("select %s from ts.Foo test INNER JOIN IdSet(?) v ON test.%s = v.id group by %s", str.c_str(), str.c_str(), str.c_str())));
            ASSERT_EQ(ECSqlStatus::Success, stmt_With_WhereClause.Prepare(m_ecdb,SqlPrintfString("select %s from IdSet(?), ts.Foo where %s = id group by %s", str.c_str(), str.c_str(), str.c_str())));
            
            IECSqlBinder& binderForJoin = stmt_With_Join.GetBinder(1);
            for(int i = 0;i<2000;i++)
                {
                int randNum = i+1;
                ASSERT_EQ(ECSqlStatus::Success, binderForJoin.AddArrayElement().BindInt64(randNum));
                emptyIdset.insert(BeInt64Id(randNum));
                }

            IECSqlBinder& binderForWhereClause = stmt_With_WhereClause.GetBinder(1);
            for(int i = 0;i<2000;i++)
                {
                int randNum = i+1;
                ASSERT_EQ(ECSqlStatus::Success, binderForWhereClause.AddArrayElement().BindInt64(randNum));
                } 

            auto iterator_set = emptyIdset.begin();
            int i = 0;
            StopWatch timer_Join_Query(true);
            while(stmt_With_Join.Step() == BE_SQLITE_ROW)
                {
                ASSERT_EQ((*iterator_set).GetValue(), stmt_With_Join.GetValueInt64(0));
                ++iterator_set;
                i++;
                }
            timer_Join_Query.Stop();
            LOGTODB(TEST_DETAILS, timer_Join_Query.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_Join.GetECSql()));

            iterator_set = emptyIdset.begin();
            i = 0;
            StopWatch timer_Query_With_WhereClause(true);
            while(stmt_With_WhereClause.Step() == BE_SQLITE_ROW)
                {
                ASSERT_EQ((*iterator_set).GetValue(), stmt_With_WhereClause.GetValueInt64(0));
                ++iterator_set;
                i++;
                }
            timer_Query_With_WhereClause.Stop();
            LOGTODB(TEST_DETAILS, timer_Query_With_WhereClause.GetElapsedSeconds(), i, SqlPrintfString("Statement: \"%s\"", stmt_With_WhereClause.GetECSql()));
            }
        ASSERT_TRUE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
        ASSERT_FALSE(EnableECSqlExperimentalFeatures(m_ecdb, false));
        }
END_ECDBUNITTESTS_NAMESPACE