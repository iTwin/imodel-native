/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "PerformanceTests.h"
#include <random>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct PerformanceRegularVsOverflowTestFixture : ECDbTestFixture
    {
    enum class Scenario
        {
        Regular,
        Overflow
        };

    static const int s_initialInstanceCount = 1000;
    static const int64_t s_firstInstanceId = 2000000;
    static const int s_insertCount = 100;
    static const int s_readCount = 500;


    void RunIntegerTest(int intValues[], Scenario, bool getReadTime);
    void RunLongTest(int64_t longValues[], Scenario, bool getReadTime);
    void RunDoubleTest(double doubleValues[], Scenario, bool getReadTime);
    void RunBoolTest(bool boolValues[], Scenario, bool getReadTime);
    void RunStringTest(Utf8String stringValues[], Scenario, bool getReadTime);
    void RunPoint2dTest(std::vector<DPoint2d> const& point2dValues, Scenario, bool getReadTime);
    void RunPoint3dTest(std::vector<DPoint3d> const& point3dValues, Scenario, bool getReadTime);
    void RunBlobTest(Utf8String stringValues[], Scenario, bool getReadTime);
    void SetUpTestDb(Utf8String seedDbName, Utf8CP schemaXml, Utf8String destFileName);

    void RunIntegerTestSpecifiedSchema(int intValues[], size_t propertiesCount, int maxSharedColumnsBeforeOverflow, bool getReadTime);
    void SetUpDbWithSpecifiedSchema(Utf8String seedDbName, size_t propertiesCount, int maxSharedColumnsBeforeOverflow);
    void GetECSqlStatements(Utf8StringR insertECSql, Utf8StringR selectECSql);

    void ReopenECDb();

    void GetRandomIntegers(int intValues[], int count);
    void GetRandomStrings(Utf8String stringValues[]);

    static void GetTestSchemaXml(Utf8StringR schemaXml, PrimitiveType, Scenario);
    static int DetermineECInstanceIdIncrement(int initialInstanceCount, int opCount) { return initialInstanceCount / opCount; }

    static Utf8CP PrimitiveTypeToXmlString(PrimitiveType);
    static Utf8CP ScenarioToString(Scenario scenario) { return scenario == Scenario::Regular ? "regular" : "overflow"; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunIntegerTest(int intValues[], Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindInt(2, intValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Integer_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());

        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId=?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueInt(0), intValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Integer_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunLongTest(int64_t longValues[], Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindInt64(2, longValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Long_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());

        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId=?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueInt64(0), longValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Long_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunDoubleTest(double doubleValues[], Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindDouble(2, doubleValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Double_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            //printf("expected: %f\n", statement.GetValueDouble(0));
            //printf("actual: %f\n", doubleValues[i*instanceIdIncrement]);

            ASSERT_NEAR(statement.GetValueDouble(0), doubleValues[i*instanceIdIncrement], 1.99e+293);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Double_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunBoolTest(bool boolValues[], Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindBoolean(2, boolValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Bool_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueBoolean(0), boolValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Bool_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunStringTest(Utf8String stringValues[], Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindText(2, stringValues[i].c_str(), IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Text_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueText(0), stringValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Text_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunPoint2dTest(std::vector<DPoint2d> const& point2dValues, Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindPoint2d(2, point2dValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Point2d_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());

        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_TRUE(point2dValues[i*instanceIdIncrement].AlmostEqual(statement.GetValuePoint2d(0))) << "Row " << i << " difference: " << statement.GetValuePoint2d(0).Distance(point2dValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Point2d_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunPoint3dTest(std::vector<DPoint3d> const& point3dValues, Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindPoint3d(2, point3dValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Point3d_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_TRUE(point3dValues[i*instanceIdIncrement].AlmostEqual(statement.GetValuePoint3d(0))) << "Row " << i << " difference: " << statement.GetValuePoint3d(0).Distance(point3dValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Point3d_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunBlobTest(Utf8String stringValues[], Scenario scenario, bool getReadTime)
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.TestClass(ECInstanceId, Prop) VALUES(?,?)"));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindBlob(2, stringValues[i].c_str(), (int) stringValues[i].length() + 1, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    Utf8String testDescription;
    testDescription.Sprintf("Binary_Insert_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
    statement.Finalize();

    if (getReadTime)
        {
        ReopenECDb();
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Prop FROM ts.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            int actualBlobSize = -1;
            EXPECT_STREQ(stringValues[i*instanceIdIncrement].c_str(), (Utf8CP) statement.GetValueBlob(0, &actualBlobSize));
            EXPECT_EQ((int) stringValues[i*instanceIdIncrement].length() + 1, actualBlobSize);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Binary_Read_Performance_%s [Initial count: %d]", ScenarioToString(scenario), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_readCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::RunIntegerTestSpecifiedSchema(int intValues[], size_t propertiesCount, int maxSharedColumnsBeforeOverflow, bool getReadTime)
    {
    Utf8String insertECSql;
    Utf8String selectECSql;
    GetECSqlStatements(insertECSql, selectECSql);

    StopWatch timer(true);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, insertECSql.c_str()));
    for (int i = 0; i < s_insertCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        for (size_t parameterIndex = 2; parameterIndex <= (propertiesCount + 1); parameterIndex++)
            {
            if (!statement.BindInt((int)parameterIndex, intValues[i]).IsSuccess())
                {
                ASSERT_TRUE(false);
                return;
                }
            }
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();
    statement.Finalize();

    Utf8String testDescription;
    testDescription.Sprintf("Insert_IntProps [MaxSharedColumnsBeforeOverflow : %d] [PropertiesCount : %d]", maxSharedColumnsBeforeOverflow, propertiesCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);
        //printf("SelectECSql : %s\n", selectECSql.c_str());
        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, selectECSql.c_str()));
        for (int i = 0; i < s_readCount; i++)
            {
            if (!statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement).IsSuccess())
                {
                FAIL();
                return;
                }

            DbResult stat = statement.Step();
            if (stat != BE_SQLITE_ROW && stat != BE_SQLITE_DONE)
                {
                FAIL();
                return;
                }

            int columnCount = statement.GetColumnCount();
            ASSERT_EQ((int) propertiesCount, columnCount);

            for (size_t parameterIndex = 0; parameterIndex < propertiesCount; parameterIndex++)
                {
                //printf("intValues[i*instanceIdIncrement] : %d\n", intValues[i*instanceIdIncrement]);
                //printf("statement.GetValueInt(parameterIndex) : %d\n", statement.GetValueInt(parameterIndex));
                if (intValues[i*instanceIdIncrement] != statement.GetValueInt((int)parameterIndex))
                    {
                    ASSERT_TRUE(false);
                    return;
                    }
                }

            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();
        statement.Finalize();

        testDescription.Sprintf("Read_IntProps [MaxSharedColumnsBeforeOverflow : %d] [PropertiesCount : %d]", maxSharedColumnsBeforeOverflow, propertiesCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_insertCount, testDescription.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, IntegerPerformance)
    {
    // get random int values between minimum and maximum integer
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    int intValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        intValues[i] = dist(rd);

    Utf8String fileName;
    fileName.Sprintf("int_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Integer, Scenario::Regular);
    SetUpTestDb(fileName, testSchemaXml.c_str(), "intinsertperformanceregularcolumn.ecdb");

    RunIntegerTest(intValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    fileName.Sprintf("int_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Integer, Scenario::Overflow);

    SetUpTestDb(fileName, testSchemaXml.c_str(), "intinsertperformanceoverflowcolumn.ecdb");

    RunIntegerTest(intValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, LongPerformance)
    {
    // get random int values between minimum and maximum long
    std::random_device rd;
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<int64_t>().min(), std::numeric_limits<int64_t>().max());
    int64_t longValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        longValues[i] = dist(rd);

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Long, Scenario::Regular);

    Utf8String fileName;
    fileName.Sprintf("long_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "longinsertperformanceregularcolumn.ecdb");

    RunLongTest(longValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    fileName.Sprintf("long_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Long, Scenario::Overflow);
    SetUpTestDb(fileName, testSchemaXml.c_str(), "longinsertperformanceoverflowcolumn.ecdb");

    RunLongTest(longValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, DoublePerformance)
    {
    // get random int values between minimum and maximum Double
    std::random_device rd;
    double doubleValLimit = std::numeric_limits<double>().max() / 2;
    std::uniform_real_distribution<double> dist(-doubleValLimit, doubleValLimit);
    double doubleValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        doubleValues[i] = dist(rd);

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Long, Scenario::Regular);

    Utf8String fileName;
    fileName.Sprintf("double_regularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "doubleinsertperformanceregularcolumn.ecdb");
    ASSERT_TRUE(m_ecdb.IsDbOpen());
    RunDoubleTest(doubleValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    fileName.Sprintf("double_overflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Long, Scenario::Overflow);
    SetUpTestDb(fileName, testSchemaXml.c_str(), "doubleinsertperformanceoverflowcolumn.ecdb");

    RunDoubleTest(doubleValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, BoolPerformance)
    {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    bool boolValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        boolValues[i] = dist(rd) % 2 != 0;

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Boolean, Scenario::Regular);

    Utf8String fileName;
    fileName.Sprintf("bool_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "boolinsertperformanceregularcolumn.ecdb");

    RunBoolTest(boolValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    fileName.Sprintf("bool_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Boolean, Scenario::Overflow);
    SetUpTestDb(fileName, testSchemaXml.c_str(), "boolinsertperformanceoverflowcolumn.ecdb");

    RunBoolTest(boolValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, StringPerformance)
    {
    Utf8String stringValues[s_insertCount];
    GetRandomStrings(stringValues);

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_String, Scenario::Regular);

    Utf8String fileName;
    fileName.Sprintf("string_regularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "stringinsertperformanceregularcolumn.ecdb");

    RunStringTest(stringValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_String, Scenario::Overflow);

    fileName.Sprintf("string_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "stringinsertperformanceoverflowcolumn.ecdb");

    RunStringTest(stringValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, Point2dPerformance)
    {
    std::random_device rd;
    std::uniform_real_distribution<double> dist(-10000.0, 10000.0);
    std::vector<DPoint2d> point2dValues;
    for (int i = 0; i < s_insertCount; i++)
        point2dValues.push_back(DPoint2d::From(dist(rd), dist(rd)));

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Point2d, Scenario::Regular);

    Utf8String fileName;
    fileName.Sprintf("point2d_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "point2dinsertperformanceregularcolumn.ecdb");

    RunPoint2dTest(point2dValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Point2d, Scenario::Overflow);

    fileName.Sprintf("point2d_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "point2dinsertperformanceoverflowcolumn.ecdb");

    RunPoint2dTest(point2dValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, Point3dPerformance)
    {
    std::random_device rd;
    std::uniform_real_distribution<double> dist(-10000.0, 10000.0);
    std::vector<DPoint3d> point3dValues;
    for (int i = 0; i < s_insertCount; i++)
        point3dValues.push_back(DPoint3d::From(dist(rd), dist(rd), dist(rd)));

    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Point3d, Scenario::Regular);

    Utf8String fileName;
    fileName.Sprintf("point3d_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "point3dinsertperformanceregularcolumn.ecdb");

    RunPoint3dTest(point3dValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Point3d, Scenario::Overflow);

    fileName.Sprintf("point3d_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "point3dinsertperformanceoverflowcolumn.ecdb");

    RunPoint3dTest(point3dValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, BlobPerformance)
    {
    Utf8String testSchemaXml;
    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Binary, Scenario::Regular);

    Utf8String stringValues[s_insertCount];
    GetRandomStrings(stringValues);

    Utf8String fileName;
    fileName.Sprintf("binary_%s_opcount_%d_seed_%d.ecdb", ScenarioToString(Scenario::Regular), s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "binaryinsertperformanceregularcolumn.ecdb");

    RunBlobTest(stringValues, Scenario::Regular, true);
    m_ecdb.CloseDb();

    GetTestSchemaXml(testSchemaXml, PRIMITIVETYPE_Binary, Scenario::Overflow);

    fileName.Sprintf("binary_%s_opcount_%d_seed_%d.ecdb", ScenarioToString(Scenario::Overflow), s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, testSchemaXml.c_str(), "binaryinsertperformanceoverflowcolumn.ecdb");

    RunBlobTest(stringValues, Scenario::Overflow, true);
    m_ecdb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverflowTestFixture, IntegerPerformance_VaryProperties_VarySharedColumns)
    {
    int intValues[s_insertCount];
    GetRandomIntegers(intValues, s_insertCount);

    Utf8String dbName;

    // varying properties count from [0-60]
    // all properties mapping to physical Columns
    for (int propertyCount = 1; propertyCount <= 60; propertyCount++)
        {
        dbName.Sprintf("RegularVsOverflow_%d-SharedColumns_%d-Properties.ecdb", 0, propertyCount);
        SetUpDbWithSpecifiedSchema(dbName.c_str(), propertyCount, 0);
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        RunIntegerTestSpecifiedSchema(intValues, propertyCount, 0, true);
        m_ecdb.CloseDb();
        }

    // varying propertyCount [0-60] and columnCount [0-60]
    // all properties mapping to sharedColumns
    for (int count = 1; count <= 60; count++)
        {
        dbName.Sprintf("RegularVsOverflow_%d-SharedColumns_%d-Properties.ecdb", count + 1, count);
        SetUpDbWithSpecifiedSchema(dbName.c_str(), count, count + 1);
        ASSERT_TRUE(m_ecdb.IsDbOpen());
        RunIntegerTestSpecifiedSchema(intValues, count, count + 1, true);
        m_ecdb.CloseDb();
        }
    }

//------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/17
//+---------------+---------------+---------------+---------------+---------------+---
//static
void PerformanceRegularVsOverflowTestFixture::GetTestSchemaXml(Utf8StringR schemaXml, PrimitiveType primType, Scenario scenario)
    {
    switch (scenario)
        {
            case Scenario::Regular:
                schemaXml.Sprintf(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='RegularColumn' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "    <ECEntityClass typeName='TestClass' modifier='None'>"
                    "        <ECProperty propertyName='Prop' typeName='%s' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", PrimitiveTypeToXmlString(primType));
                return;

            case Scenario::Overflow:
                schemaXml.Sprintf(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='OverflowColumn' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                    "    <ECEntityClass typeName='TestClass' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Prop' typeName='%s' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", PrimitiveTypeToXmlString(primType));
                return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::SetUpTestDb(Utf8String seedDbName, Utf8CP schemaXml, Utf8String destFileName)
    {
    BeFileName seedFilePath = BuildECDbPath(seedDbName.c_str());

    if (!seedFilePath.DoesPathExist())
        {
        SetupECDb(seedDbName.c_str(), SchemaItem(schemaXml));
        ASSERT_EQ(SUCCESS, PopulateECDb(s_initialInstanceCount));
        m_ecdb.CloseDb();
        }

    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(destFileName.c_str(), seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::SetUpDbWithSpecifiedSchema(Utf8String seedDbName, size_t propertiesCount, int maxSharedColumnsBeforeOverflow)
    {
    Utf8String seedFileName;
    seedFileName.Sprintf("RegularVsOverflow_%d-SharedColumns_%d-Properties_seed%d.ecdb", maxSharedColumnsBeforeOverflow, propertiesCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName seedFilePath = BuildECDbPath(seedFileName.c_str());

    if (!seedFilePath.DoesPathExist())
        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb(seedFileName.c_str()));

        ECSchemaPtr testSchema = nullptr;
        ECEntityClassP testClass = nullptr;

        ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 0);
        ASSERT_TRUE(testSchema.IsValid());

        ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(testClass, "TestClass"));


        ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
        readContext->AddSchema(*testSchema);

        if (maxSharedColumnsBeforeOverflow > 0)
            {
            readContext->AddSchemaLocater(m_ecdb.GetSchemaLocater());
            SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
            ECSchemaPtr ecdbMapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestWriteCompatible);
            ASSERT_TRUE(ecdbMapSchema.IsValid());
            testSchema->AddReferencedSchema(*ecdbMapSchema);

            StandaloneECInstancePtr tablePerHierarchyCA = ecdbMapSchema->GetClassCP("ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
            ASSERT_TRUE(tablePerHierarchyCA != nullptr);
            ASSERT_TRUE(tablePerHierarchyCA->SetValue("MapStrategy", ECValue("TablePerHierarchy")) == ECObjectsStatus::Success);
            ASSERT_TRUE(testClass->SetCustomAttribute(*tablePerHierarchyCA) == ECObjectsStatus::Success);

            StandaloneECInstancePtr sharedColumnCA = ecdbMapSchema->GetClassCP("ShareColumns")->GetDefaultStandaloneEnabler()->CreateInstance();
            ASSERT_TRUE(sharedColumnCA != nullptr);
            ASSERT_TRUE(sharedColumnCA->SetValue("MaxSharedColumnsBeforeOverflow", ECValue(maxSharedColumnsBeforeOverflow)) == ECObjectsStatus::Success);
            ASSERT_TRUE(testClass->SetCustomAttribute(*sharedColumnCA) == ECObjectsStatus::Success);
            }

        PrimitiveECPropertyP primitiveP;
        for (size_t i = 0; i < propertiesCount; i++)
            {
            Utf8String temp;
            temp.Sprintf("_Property%d", i);
            Utf8String propName = testClass->GetName().c_str();
            propName.append(temp);
            testClass->CreatePrimitiveProperty(primitiveP, propName, PrimitiveType::PRIMITIVETYPE_Integer);
            }

        EXPECT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
        EXPECT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

        m_ecdb.CloseDb();
        }

    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(seedDbName.c_str(), seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::GetECSqlStatements(Utf8StringR insertECSql, Utf8StringR selectECSql)
    {
    ECSchemaCP ecSchema = m_ecdb.Schemas().GetSchema("TestSchema", true);
    for (ECClassCP testClass : ecSchema->GetClasses())
        {
        Utf8StringCR className = testClass->GetECSqlName();

        insertECSql = Utf8String("INSERT INTO ");
        insertECSql.append(className).append(" (ECInstanceId, ");
        Utf8String insertValuesSql(") VALUES (?, ");

        selectECSql = Utf8String("SELECT ");

        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties(true))
            {
            if (!isFirstItem)
                {
                insertECSql.append(", ");
                insertValuesSql.append(", ");

                selectECSql.append(", ");
                }

            insertECSql.append(prop->GetName());
            insertValuesSql.append("?");

            selectECSql.append(prop->GetName());

            isFirstItem = false;
            }

        insertECSql.append(insertValuesSql).append(")");

        selectECSql.append(" FROM ONLY ").append(className).append(" WHERE ECInstanceId=?");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::ReopenECDb()
    {
    Utf8String dbFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbFileName.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::GetRandomStrings(Utf8String stringValues[])
    {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 61);

    static Utf8String charSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int j = 0; j < s_insertCount; j++)
        {
        Utf8String string;
        int stringLength = dist(rd);
        string.resize(stringLength);
        for (int i = 0; i < stringLength; i++)
            string[i] = charSet[dist(rd)];

        stringValues[j] = string;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverflowTestFixture::GetRandomIntegers(int intValues[], int count)
    {
    // get random int values between minimum and maximum integer
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    for (int i = 0; i < count; i++)
        intValues[i] = dist(rd);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP PerformanceRegularVsOverflowTestFixture::PrimitiveTypeToXmlString(PrimitiveType type)
    {
    switch (type)
        {
            case PRIMITIVETYPE_Binary:
                return "binary";

            case PRIMITIVETYPE_Boolean:
                return "boolean";

            case PRIMITIVETYPE_DateTime:
                return "datetime";

            case PRIMITIVETYPE_Double:
                return "double";

            case PRIMITIVETYPE_IGeometry:
                return "Bentley.Geometry.Common.IGeometry";

            case PRIMITIVETYPE_Integer:
                return "int";

            case PRIMITIVETYPE_Long:
                return "long";

            case PRIMITIVETYPE_Point2d:
                return "point2d";

            case PRIMITIVETYPE_Point3d:
                return "point3d";

            case PRIMITIVETYPE_String:
                return "string";

            default:
                BeAssert(false);
                return "";
        }
    }
END_ECDBUNITTESTS_NAMESPACE
