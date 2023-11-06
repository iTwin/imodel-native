/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <Bentley/Base64Utilities.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlStatementWindowFunctionTestFixture : ECDbTestFixture
    {
    protected:
        void SetUp() override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementWindowFunctionTestFixture::SetUp()
    {
    ECDbTestFixture::SetUp();
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("WindowFunctionTests.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="SomeEntity" >
                <ECProperty propertyName="Primary" typeName="string" />
                <ECProperty propertyName="Secondary" typeName="string" />
                <ECProperty propertyName="SomeValue" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"
    )));

    if ("Insert data")
        {
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('A', 'one', 1)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('B', 'two', 2)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('C', 'three', 3)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('D', 'one', 4)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('E', 'two', 5)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('F', 'three', 6)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('G', 'one', 7)"));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, RowNumber)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, row_number() over(PARTITION BY Secondary ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"A","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":1,"Secondary":"one"},
            {"Primary":"D","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":2,"Secondary":"one"},
            {"Primary":"G","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":3,"Secondary":"one"},
            {"Primary":"C","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":1,"Secondary":"three"},
            {"Primary":"F","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":2,"Secondary":"three"},
            {"Primary":"B","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":1,"Secondary":"two"},
            {"Primary":"E","ROW_NUMBER() OVER(PARTITION BY [Secondary] ORDER BY [Primary])":2,"Secondary":"two"}
        ]))json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, Rank)
    {
    Utf8CP ecsql = "SELECT Secondary, rank() over(ORDER BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"RANK() OVER(ORDER BY [Secondary])":1,"Secondary":"one"},
            {"RANK() OVER(ORDER BY [Secondary])":1,"Secondary":"one"},
            {"RANK() OVER(ORDER BY [Secondary])":1,"Secondary":"one"},
            {"RANK() OVER(ORDER BY [Secondary])":4,"Secondary":"three"},
            {"RANK() OVER(ORDER BY [Secondary])":4,"Secondary":"three"},
            {"RANK() OVER(ORDER BY [Secondary])":6,"Secondary":"two"},
            {"RANK() OVER(ORDER BY [Secondary])":6,"Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, DenseRank)
    {
    Utf8CP ecsql = "SELECT Secondary, dense_rank() over(ORDER BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":1,"Secondary":"one"},
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":1,"Secondary":"one"},
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":1,"Secondary":"one"},
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":2,"Secondary":"three"},
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":2,"Secondary":"three"},
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":3,"Secondary":"two"},
            {"DENSE_RANK() OVER(ORDER BY [Secondary])":3,"Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, PercentRank)
    {
    Utf8CP ecsql = "SELECT Secondary, percent_rank() over(ORDER BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.0,"Secondary":"one"},
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.0,"Secondary":"one"},
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.0,"Secondary":"one"},
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.50,"Secondary":"three"},
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.50,"Secondary":"three"},
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.83333333333333337,"Secondary":"two"},
            {"PERCENT_RANK() OVER(ORDER BY [Secondary])":0.83333333333333337,"Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, CumeDist)
    {
    Utf8CP ecsql = "SELECT Secondary, cume_dist() over(ORDER BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"CUME_DIST() OVER(ORDER BY [Secondary])":0.42857142857142855,"Secondary":"one"},
            {"CUME_DIST() OVER(ORDER BY [Secondary])":0.42857142857142855,"Secondary":"one"},
            {"CUME_DIST() OVER(ORDER BY [Secondary])":0.42857142857142855,"Secondary":"one"},
            {"CUME_DIST() OVER(ORDER BY [Secondary])":0.71428571428571430,"Secondary":"three"},
            {"CUME_DIST() OVER(ORDER BY [Secondary])":0.71428571428571430,"Secondary":"three"},
            {"CUME_DIST() OVER(ORDER BY [Secondary])":1.0,"Secondary":"two"},
            {"CUME_DIST() OVER(ORDER BY [Secondary])":1.0,"Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, Ntile)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, ntile(3) over(ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"A","Secondary":"one","ntile(3) OVER(ORDER BY [Primary])":1},
            {"Primary":"B","Secondary":"two","ntile(3) OVER(ORDER BY [Primary])":1},
            {"Primary":"C","Secondary":"three","ntile(3) OVER(ORDER BY [Primary])":1},
            {"Primary":"D","Secondary":"one","ntile(3) OVER(ORDER BY [Primary])":2},
            {"Primary":"E","Secondary":"two","ntile(3) OVER(ORDER BY [Primary])":2},
            {"Primary":"F","Secondary":"three","ntile(3) OVER(ORDER BY [Primary])":3},
            {"Primary":"G","Secondary":"one","ntile(3) OVER(ORDER BY [Primary])":3}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, Lag)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, lag(Primary, 1, 321) over(PARTITION BY Secondary ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"321","Primary":"A","Secondary":"one"},
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"A","Primary":"D","Secondary":"one"},
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"D","Primary":"G","Secondary":"one"},
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"321","Primary":"C","Secondary":"three"},
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"C","Primary":"F","Secondary":"three"},
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"321","Primary":"B","Secondary":"two"},
            {"LAG([Primary],1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"B","Primary":"E","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, Lead)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, lead(123, 1, 321) over(PARTITION BY Secondary ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":123,"Primary":"A","Secondary":"one"},
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":123,"Primary":"D","Secondary":"one"},
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":321,"Primary":"G","Secondary":"one"},
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":123,"Primary":"C","Secondary":"three"},
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":321,"Primary":"F","Secondary":"three"},
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":123,"Primary":"B","Secondary":"two"},
            {"LEAD(123,1,321) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":321,"Primary":"E","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, FirstValue)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, first_value(Primary) over(PARTITION BY Secondary ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"A","Primary":"A","Secondary":"one"},
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"A","Primary":"D","Secondary":"one"},
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"A","Primary":"G","Secondary":"one"},
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"C","Primary":"C","Secondary":"three"},
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"C","Primary":"F","Secondary":"three"},
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"B","Primary":"B","Secondary":"two"},
            {"FIRST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"B","Primary":"E","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, LastValue)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, last_value(Primary) over(PARTITION BY Secondary ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"A","Primary":"A","Secondary":"one"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"D","Primary":"D","Secondary":"one"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"G","Primary":"G","Secondary":"one"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"C","Primary":"C","Secondary":"three"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"F","Primary":"F","Secondary":"three"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"B","Primary":"B","Secondary":"two"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"E","Primary":"E","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, NthValue)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, nth_Value(Primary, 1+1) over(PARTITION BY Secondary ORDER BY Primary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"A","Secondary":"one"},
            {"NTH_VALUE([Primary],1 + 1) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"D","Primary":"D","Secondary":"one"},
            {"NTH_VALUE([Primary],1 + 1) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"D","Primary":"G","Secondary":"one"},
            {"Primary":"C","Secondary":"three"},
            {"NTH_VALUE([Primary],1 + 1) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"F","Primary":"F","Secondary":"three"},
            {"Primary":"B","Secondary":"two"},
            {"NTH_VALUE([Primary],1 + 1) OVER(PARTITION BY [Secondary] ORDER BY [Primary])":"E","Primary":"E","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, AggregateFunction)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, MAX(SomeValue) over(PARTITION BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":7,"Primary":"A","Secondary":"one","SomeValue":1},
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":7,"Primary":"D","Secondary":"one","SomeValue":4},
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":7,"Primary":"G","Secondary":"one","SomeValue":7},
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":6,"Primary":"C","Secondary":"three","SomeValue":3},
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":6,"Primary":"F","Secondary":"three","SomeValue":6},
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":5,"Primary":"B","Secondary":"two","SomeValue":2},
            {"MAX([SomeValue]) OVER(PARTITION BY [Secondary])":5,"Primary":"E","Secondary":"two","SomeValue":5}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, AggregateFunctionWithFilter)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, MAX(SomeValue) FILTER (WHERE Primary <> 'G' AND Primary <> 'F') over(PARTITION BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":4,"Primary":"A","Secondary":"one","SomeValue":1},
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":4,"Primary":"D","Secondary":"one","SomeValue":4},
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":4,"Primary":"G","Secondary":"one","SomeValue":7},
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":3,"Primary":"C","Secondary":"three","SomeValue":3},
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":3,"Primary":"F","Secondary":"three","SomeValue":6},
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":5,"Primary":"B","Secondary":"two","SomeValue":2},
            {"MAX([SomeValue]) FILTER(WHERE [Primary] <> 'G' AND [Primary] <> 'F') OVER(PARTITION BY [Secondary])":5,"Primary":"E","Secondary":"two","SomeValue":5}]
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, RowsWithFrameStart)
    {
    Utf8CP ecsql = "SELECT Primary, first_value(Primary) over(ORDER BY Primary ROWS CURRENT ROW) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"A","Primary":"A"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"B","Primary":"B"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"C","Primary":"C"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"D","Primary":"D"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"E","Primary":"E"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"F","Primary":"F"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS CURRENT ROW)":"G","Primary":"G"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, RowsWithFrameBetween)
    {
    Utf8CP ecsql = "SELECT Primary, last_value(Primary) over(ORDER BY Primary ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"A","Primary":"A"},
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"B","Primary":"B"},
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"C","Primary":"C"},
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"D","Primary":"D"},
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"E","Primary":"E"},
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"F","Primary":"F"},
            {"LAST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"G","Primary":"G"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, GroupsWithFrameStart)
    {
    Utf8CP ecsql = "SELECT Secondary, last_value(Secondary) over(ORDER BY Secondary GROUPS UNBOUNDED PRECEDING) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"one","Secondary":"one"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"one","Secondary":"one"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"one","Secondary":"one"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"three","Secondary":"three"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"three","Secondary":"three"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"two","Secondary":"two"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS UNBOUNDED PRECEDING)":"two","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, GroupsWithFrameBetween)
    {
    Utf8CP ecsql = "SELECT Secondary, last_value(Secondary) over(ORDER BY Secondary GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"one"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"one"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"one"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"three"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"three"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"two"},
            {"LAST_VALUE([Secondary]) OVER(ORDER BY [Secondary] GROUPS BETWEEN 2 + 2 PRECEDING AND 2 + 3 FOLLOWING)":"two","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, RangeWithFrameStart)
    {
    Utf8CP ecsql = "SELECT Secondary, first_value(Secondary) over(ORDER BY Secondary RANGE 0 + 1 PRECEDING) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"one","Secondary":"one"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"one","Secondary":"one"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"one","Secondary":"one"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"three","Secondary":"three"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"three","Secondary":"three"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"two","Secondary":"two"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE 0 + 1 PRECEDING)":"two","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, RangeWithFrameBetween)
    {
    Utf8CP ecsql = "SELECT Secondary, first_value(Secondary) over(ORDER BY Secondary RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"one","Secondary":"one"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"one","Secondary":"one"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"one","Secondary":"one"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"three","Secondary":"three"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"three","Secondary":"three"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"two","Secondary":"two"},
            {"FIRST_VALUE([Secondary]) OVER(ORDER BY [Secondary] RANGE BETWEEN 0 + 1 PRECEDING AND CURRENT ROW)":"two","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, ExcludeNoOthers)
    {
    Utf8CP ecsql = "SELECT Primary, first_value(Primary) over(ORDER BY Primary ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"A","Primary":"A"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"A","Primary":"B"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"A","Primary":"C"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"B","Primary":"D"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"C","Primary":"E"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"D","Primary":"F"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)":"E","Primary":"G"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, ExcludeCurrentRow)
    {
    Utf8CP ecsql = "SELECT Primary, first_value(Primary) over(ORDER BY Primary ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"B","Primary":"A"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"A","Primary":"B"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"A","Primary":"C"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"B","Primary":"D"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"C","Primary":"E"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"D","Primary":"F"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN 2 PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW)":"E","Primary":"G"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, ExcludeGroup)
    {
    Utf8CP ecsql = "SELECT Primary, first_value(Primary) over(ORDER BY Primary ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP)":"B","Primary":"A"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP)":"C","Primary":"B"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP)":"D","Primary":"C"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP)":"E","Primary":"D"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP)":"F","Primary":"E"},
            {"FIRST_VALUE([Primary]) OVER(ORDER BY [Primary] ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING EXCLUDE GROUP)":"G","Primary":"F"},
            {"Primary":"G"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, ExcludeTies)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, last_value(Primary) over(PARTITION BY Secondary ROWS UNBOUNDED PRECEDING EXCLUDE TIES) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"A","Secondary":"one"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ROWS UNBOUNDED PRECEDING EXCLUDE TIES)":"A","Primary":"D","Secondary":"one"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ROWS UNBOUNDED PRECEDING EXCLUDE TIES)":"D","Primary":"G","Secondary":"one"},
            {"Primary":"C","Secondary":"three"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ROWS UNBOUNDED PRECEDING EXCLUDE TIES)":"C","Primary":"F","Secondary":"three"},
            {"Primary":"B","Secondary":"two"},
            {"LAST_VALUE([Primary]) OVER(PARTITION BY [Secondary] ROWS UNBOUNDED PRECEDING EXCLUDE TIES)":"B","Primary":"E","Secondary":"two"}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, WithWindowName)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, MAX(SomeValue) over win, MIN(SomeValue) over win from ts.SomeEntity WINDOW win AS (PARTITION BY Secondary)";
    auto expected = JsonValue(R"json([
            {"MAX([SomeValue]) OVER win":7,"MIN([SomeValue]) OVER win":1,"Primary":"A","Secondary":"one","SomeValue":1},
            {"MAX([SomeValue]) OVER win":7,"MIN([SomeValue]) OVER win":1,"Primary":"D","Secondary":"one","SomeValue":4},
            {"MAX([SomeValue]) OVER win":7,"MIN([SomeValue]) OVER win":1,"Primary":"G","Secondary":"one","SomeValue":7},
            {"MAX([SomeValue]) OVER win":6,"MIN([SomeValue]) OVER win":3,"Primary":"C","Secondary":"three","SomeValue":3},
            {"MAX([SomeValue]) OVER win":6,"MIN([SomeValue]) OVER win":3,"Primary":"F","Secondary":"three","SomeValue":6},
            {"MAX([SomeValue]) OVER win":5,"MIN([SomeValue]) OVER win":2,"Primary":"B","Secondary":"two","SomeValue":2},
            {"MAX([SomeValue]) OVER win":5,"MIN([SomeValue]) OVER win":2,"Primary":"E","Secondary":"two","SomeValue":5}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, WithWindowChaining)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, group_concat(Primary) over (win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) from ts.SomeEntity WINDOW win AS (PARTITION BY Secondary ORDER BY SomeValue)";
    auto expected = JsonValue(R"json([
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"A","Primary":"A","Secondary":"one","SomeValue":1},
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"A,D","Primary":"D","Secondary":"one","SomeValue":4},
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"A,D,G","Primary":"G","Secondary":"one","SomeValue":7},
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"C","Primary":"C","Secondary":"three","SomeValue":3},
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"C,F","Primary":"F","Secondary":"three","SomeValue":6},
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"B","Primary":"B","Secondary":"two","SomeValue":2},
            {"GROUP_CONCAT([Primary]) OVER(win ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)":"B,E","Primary":"E","Secondary":"two","SomeValue":5}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowFunctionTestFixture, WithMultipleWindowNames)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, MIN(SomeValue) over win1, MIN(SomeValue) over win2 from ts.SomeEntity WINDOW win1 AS (PARTITION BY Secondary), win2 AS(PARTITION BY Primary)";
    auto expected = JsonValue(R"json([
            {"MIN([SomeValue]) OVER win1":1,"MIN([SomeValue]) OVER win2":1,"Primary":"A","Secondary":"one","SomeValue":1},
            {"MIN([SomeValue]) OVER win1":1,"MIN([SomeValue]) OVER win2":4,"Primary":"D","Secondary":"one","SomeValue":4},
            {"MIN([SomeValue]) OVER win1":1,"MIN([SomeValue]) OVER win2":7,"Primary":"G","Secondary":"one","SomeValue":7},
            {"MIN([SomeValue]) OVER win1":3,"MIN([SomeValue]) OVER win2":3,"Primary":"C","Secondary":"three","SomeValue":3},
            {"MIN([SomeValue]) OVER win1":3,"MIN([SomeValue]) OVER win2":6,"Primary":"F","Secondary":"three","SomeValue":6},
            {"MIN([SomeValue]) OVER win1":2,"MIN([SomeValue]) OVER win2":2,"Primary":"B","Secondary":"two","SomeValue":2},
            {"MIN([SomeValue]) OVER win1":2,"MIN([SomeValue]) OVER win2":5,"Primary":"E","Secondary":"two","SomeValue":5}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

struct ECSqlStatementWindowPartitionTestFixture : ECDbTestFixture
    {
    protected:
        void SetUp();
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementWindowPartitionTestFixture::SetUp()
    {
    ECDbTestFixture::SetUp();
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("WindowPartitionClause.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="SomeEntity" >
                <ECProperty propertyName="Primary" typeName="string" />
                <ECProperty propertyName="Secondary" typeName="string" />
                <ECProperty propertyName="SomeValue" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"
    )));

    if ("Insert data")
        {
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('A', 'one', 1)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('B', 'two', 2)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('C', 'three', 3)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('D', 'one ', 4)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('E', 'two', 5)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('F', 'three', 6)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('G', 'ONE', 1)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('H', 'two', 2)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SomeEntity(Primary,Secondary,SomeValue) VALUES ('I', 'three', 3)"));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowPartitionTestFixture, WindowPartitionClause)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, row_number() over(PARTITION BY Secondary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"G","ROW_NUMBER() OVER(PARTITION BY [Secondary])":1,"Secondary":"ONE","SomeValue":1},
            {"Primary":"A","ROW_NUMBER() OVER(PARTITION BY [Secondary])":1,"Secondary":"one","SomeValue":1},
            {"Primary":"D","ROW_NUMBER() OVER(PARTITION BY [Secondary])":1,"Secondary":"one ","SomeValue":4},
            {"Primary":"C","ROW_NUMBER() OVER(PARTITION BY [Secondary])":1,"Secondary":"three","SomeValue":3},
            {"Primary":"F","ROW_NUMBER() OVER(PARTITION BY [Secondary])":2,"Secondary":"three","SomeValue":6},
            {"Primary":"I","ROW_NUMBER() OVER(PARTITION BY [Secondary])":3,"Secondary":"three","SomeValue":3},
            {"Primary":"B","ROW_NUMBER() OVER(PARTITION BY [Secondary])":1,"Secondary":"two","SomeValue":2},
            {"Primary":"E","ROW_NUMBER() OVER(PARTITION BY [Secondary])":2,"Secondary":"two","SomeValue":5},
            {"Primary":"H","ROW_NUMBER() OVER(PARTITION BY [Secondary])":3,"Secondary":"two","SomeValue":2}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowPartitionTestFixture, WindowPartitionClauseWithRtrim)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, row_number() over(PARTITION BY Secondary collate rtrim) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"G","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":1,"Secondary":"ONE","SomeValue":1},
            {"Primary":"A","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":1,"Secondary":"one","SomeValue":1},
            {"Primary":"D","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":2,"Secondary":"one ","SomeValue":4},
            {"Primary":"C","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":1,"Secondary":"three","SomeValue":3},
            {"Primary":"F","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":2,"Secondary":"three","SomeValue":6},
            {"Primary":"I","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":3,"Secondary":"three","SomeValue":3},
            {"Primary":"B","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":1,"Secondary":"two","SomeValue":2},
            {"Primary":"E","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":2,"Secondary":"two","SomeValue":5},
            {"Primary":"H","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE RTRIM)":3,"Secondary":"two","SomeValue":2}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowPartitionTestFixture, WindowPartitionClauseWithNoCase)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, row_number() over(PARTITION BY Secondary collate nocase) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"A","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":1,"Secondary":"one","SomeValue":1},
            {"Primary":"G","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":2,"Secondary":"ONE","SomeValue":1},
            {"Primary":"D","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":1,"Secondary":"one ","SomeValue":4},
            {"Primary":"C","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":1,"Secondary":"three","SomeValue":3},
            {"Primary":"F","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":2,"Secondary":"three","SomeValue":6},
            {"Primary":"I","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":3,"Secondary":"three","SomeValue":3},
            {"Primary":"B","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":1,"Secondary":"two","SomeValue":2},
            {"Primary":"E","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":2,"Secondary":"two","SomeValue":5},
            {"Primary":"H","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE)":3,"Secondary":"two","SomeValue":2}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowPartitionTestFixture, WindowPartitionClauseWithBinary)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, row_number() over(PARTITION BY Secondary collate binary) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"G","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":1,"Secondary":"ONE","SomeValue":1},
            {"Primary":"A","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":1,"Secondary":"one","SomeValue":1},
            {"Primary":"D","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":1,"Secondary":"one ","SomeValue":4},
            {"Primary":"C","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":1,"Secondary":"three","SomeValue":3},
            {"Primary":"F","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":2,"Secondary":"three","SomeValue":6},
            {"Primary":"I","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":3,"Secondary":"three","SomeValue":3},
            {"Primary":"B","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":1,"Secondary":"two","SomeValue":2},
            {"Primary":"E","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":2,"Secondary":"two","SomeValue":5},
            {"Primary":"H","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE BINARY)":3,"Secondary":"two","SomeValue":2}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementWindowPartitionTestFixture, MultipleWindowPartitionClauses)
    {
    Utf8CP ecsql = "SELECT Primary, Secondary, SomeValue, row_number() over(PARTITION BY Secondary collate nocase, SomeValue) from ts.SomeEntity";
    auto expected = JsonValue(R"json([
            {"Primary":"A","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":1,"Secondary":"one","SomeValue":1},
            {"Primary":"G","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":2,"Secondary":"ONE","SomeValue":1},
            {"Primary":"D","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":1,"Secondary":"one ","SomeValue":4},
            {"Primary":"C","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":1,"Secondary":"three","SomeValue":3},
            {"Primary":"I","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":2,"Secondary":"three","SomeValue":3},
            {"Primary":"F","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":1,"Secondary":"three","SomeValue":6},
            {"Primary":"B","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":1,"Secondary":"two","SomeValue":2},
            {"Primary":"H","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":2,"Secondary":"two","SomeValue":2},
            {"Primary":"E","ROW_NUMBER() OVER(PARTITION BY [Secondary] COLLATE NOCASE, [SomeValue])":1,"Secondary":"two","SomeValue":5}
        ])json");
    ASSERT_EQ(expected, GetHelper().ExecuteSelectECSql(ecsql));
    }

END_ECDBUNITTESTS_NAMESPACE
