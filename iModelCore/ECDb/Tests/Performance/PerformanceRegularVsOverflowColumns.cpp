/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceRegularVsOverflowColumns.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PerformanceTests.h"
#include "random"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct PerformanceRegularVsOverFlowColumns : ECDbTestFixture
    {
    static const int s_initialInstanceCount = 1000000;
    static const int64_t s_firstInstanceId = 2000000;
    static const int s_insertCount = 100000;
    static const int s_readCount = 50000;

    void GetInsertECSql(Utf8StringR insertECSql);

    void SetUpTestDb(Utf8String seedDbName, Utf8CP schemaXml, Utf8String destFileName);

    void ReopenECDb();

    void GetRandomStrings(Utf8String stringValues[]);

    void RunIntegerTest(int intValues[], Utf8String columnType, bool getReadTime);
    void RunLongTest(int64_t longValues[], Utf8String columnType, bool getReadTime);
    void RunDoubleTest(double doubleValues[], Utf8String columnType, bool getReadTime);
    void RunBoolTest(bool boolValues[], Utf8String columnType, bool getReadTime);
    void RunStringTest(Utf8String stringValues[], Utf8String columnType, bool getReadTime);
    void RunPoint2dTest(std::vector<DPoint2d> point2dValues, Utf8String columnType, bool getReadTime);
    void RunPoint3dTest(std::vector<DPoint3d> point3dValues, Utf8String columnType, bool getReadTime);
    void RunBlobTest(Utf8String stringValues[], Utf8String columnType, bool getReadTime);

    static int DetermineECInstanceIdIncrement(int initialInstanceCount, int opCount) { return initialInstanceCount / opCount; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::GetInsertECSql(Utf8StringR insertECSql)
    {
    ECSchemaCP ecSchema = GetECDb().Schemas().GetECSchema("TestSchema", true);
    for (ECClassCP testClass : ecSchema->GetClasses())
        {
        Utf8StringCR className = testClass->GetECSqlName();

        insertECSql = Utf8String("INSERT INTO ");
        insertECSql.append(className).append(" (ECInstanceId, ");
        Utf8String insertValuesSql(") VALUES (?, ");

        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties(true))
            {
            if (!isFirstItem)
                {
                insertECSql.append(", ");
                insertValuesSql.append(", ");
                }

            insertECSql.append(prop->GetName());
            insertValuesSql.append("?");

            isFirstItem = false;
            }

        insertECSql.append(insertValuesSql).append(")");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::SetUpTestDb(Utf8String seedDbName, Utf8CP schemaXml, Utf8String destFileName)
    {
    BeFileName seedFilePath = ECDbTestUtility::BuildECDbPath(seedDbName.c_str());

    if (!seedFilePath.DoesPathExist())
        {
        SetupECDb(seedDbName.c_str(), SchemaItem(schemaXml, true), s_initialInstanceCount);

        GetECDb().SaveChanges();
        GetECDb().CloseDb();
        }


    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(m_ecdb, destFileName.c_str(), seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    //BeFileName destFilePath = ECDbTestUtility::BuildECDbPath(destFileName.c_str());
    //EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedFilePath, destFilePath));
    //EXPECT_EQ(BE_SQLITE_OK, GetECDb().OpenBeSQLiteDb(destFileName.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::ReopenECDb()
    {
    Utf8CP dbFileName = GetECDb().GetDbFileName();
    GetECDb().CloseDb();
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbFileName, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::GetRandomStrings(Utf8String stringValues[])
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
void PerformanceRegularVsOverFlowColumns::RunIntegerTest(int intValues[], Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Integer_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT IntProp FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueInt(0), intValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Integer_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunLongTest(int64_t longValues[], Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Long_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT LongProp FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueInt64(0), longValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Long_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunDoubleTest(double doubleValues[], Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Double_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DoubleProp FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
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

        testDescription.Sprintf("Double_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunBoolTest(bool boolValues[], Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Bool_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT boolProp FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueBoolean(0), boolValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Bool_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunStringTest(Utf8String stringValues[], Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Text_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT stringProp FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_EQ(statement.GetValueText(0), stringValues[i*instanceIdIncrement]);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Text_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunPoint2dTest(std::vector<DPoint2d> point2dValues, Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Point2d_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT Point2dProp.x, Point2dProp.y FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_NEAR(statement.GetValueDouble(0), point2dValues[i*instanceIdIncrement].x, 1.99e+293);
            ASSERT_NEAR(statement.GetValueDouble(1), point2dValues[i*instanceIdIncrement].y, 1.99e+293);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Point2d_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunPoint3dTest(std::vector<DPoint3d> point3dValues, Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Point3d_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT Point3dProp.x, Point3dProp.y, Point3dProp.z FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
        for (int i = 0; i < s_readCount; i++)
            {
            statement.BindInt64(1, s_firstInstanceId + i*instanceIdIncrement);
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

            ASSERT_NEAR(statement.GetValueDouble(0), point3dValues[i*instanceIdIncrement].x, 1.99e+293);
            ASSERT_NEAR(statement.GetValueDouble(1), point3dValues[i*instanceIdIncrement].y, 1.99e+293);
            ASSERT_NEAR(statement.GetValueDouble(2), point3dValues[i*instanceIdIncrement].z, 1.99e+293);
            statement.Reset();
            statement.ClearBindings();
            }
        timer.Stop();

        testDescription.Sprintf("Point3d_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunBlobTest(Utf8String stringValues[], Utf8String columnType, bool getReadTime)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
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
    testDescription.Sprintf("Binary_Insert_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_insertCount);
    statement.Finalize();

    ReopenECDb();

    if (getReadTime)
        {
        const int instanceIdIncrement = DetermineECInstanceIdIncrement(s_insertCount, s_readCount);

        timer.Start();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT BinaryProp FROM TestSchema.TestClass WHERE ECInstanceId = ?"));
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

        testDescription.Sprintf("Binary_Read_Performance_%s [Initial count: %d]", columnType.c_str(), s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_readCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, IntegerPerformance)
    {
    // get random int values between minimum and maximum integer
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    int intValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        intValues[i] = dist(rd);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='IntProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("int_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "intinsertperformanceregularcolumn.ecdb");

    RunIntegerTest(intValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='IntProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("int_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "intinsertperformanceoverflowcolumn.ecdb");

    RunIntegerTest(intValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, LongPerformance)
    {
    // get random int values between minimum and maximum long
    std::random_device rd;
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<int64_t>().min(), std::numeric_limits<int64_t>().max());
    int64_t longValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        longValues[i] = dist(rd);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='LongProp' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("long_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "longinsertperformanceregularcolumn.ecdb");

    RunLongTest(longValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='LongProp' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("long_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "longinsertperformanceoverflowcolumn.ecdb");

    RunLongTest(longValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, DoublePerformance)
    {
    // get random int values between minimum and maximum Double
    std::random_device rd;
    double doubleValLimit = std::numeric_limits<double>().max() / 2;
    std::uniform_real_distribution<double> dist(-doubleValLimit, doubleValLimit);
    double doubleValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        doubleValues[i] = dist(rd);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='DoubleProp' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("double_regularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "doubleinsertperformanceregularcolumn.ecdb");

    RunDoubleTest(doubleValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='DoubleProp' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("double_overflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "doubleinsertperformanceoverflowcolumn.ecdb");

    RunDoubleTest(doubleValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, BoolPerformance)
    {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    bool boolValues[s_insertCount];
    for (int i = 0; i < s_insertCount; i++)
        boolValues[i] = dist(rd) % 2 != 0;

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='boolProp' typeName='boolean' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("bool_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "boolinsertperformanceregularcolumn.ecdb");

    RunBoolTest(boolValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='BoolProp' typeName='boolean' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("bool_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "boolinsertperformanceoverflowcolumn.ecdb");

    RunBoolTest(boolValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, TextPerformance)
    {
    Utf8String stringValues[s_insertCount];
    GetRandomStrings(stringValues);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='stringProp' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("string_regularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "stringinsertperformanceregularcolumn.ecdb");

    RunStringTest(stringValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='stringProp' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("string_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "stringinsertperformanceoverflowcolumn.ecdb");

    RunStringTest(stringValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, Point2dPerformance)
    {
    std::random_device rd;
    double doubleValLimit = std::numeric_limits<double>().max() / 2;
    std::uniform_real_distribution<double> dist(-doubleValLimit, doubleValLimit);
    std::vector<DPoint2d> point2dValues;
    for (int i = 0; i < s_insertCount; i++)
        point2dValues.push_back(DPoint2d::From(dist(rd), dist(rd)));

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='Point2dProp' typeName='point2d' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("point2d_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "point2dinsertperformanceregularcolumn.ecdb");

    RunPoint2dTest(point2dValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Point2dProp' typeName='point2d' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("point2d_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "point2dinsertperformanceoverflowcolumn.ecdb");

    RunPoint2dTest(point2dValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, Point3dPerformance)
    {
    std::random_device rd;
    double doubleValLimit = std::numeric_limits<double>().max() / 2;
    std::uniform_real_distribution<double> dist(-doubleValLimit, doubleValLimit);
    std::vector<DPoint3d> point3dValues;
    for (int i = 0; i < s_insertCount; i++)
        point3dValues.push_back(DPoint3d::From(dist(rd), dist(rd), dist(rd)));

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='Point3dProp' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("point3d_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "point3dinsertperformanceregularcolumn.ecdb");

    RunPoint3dTest(point3dValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Point3dProp' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("point3d_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "point3dinsertperformanceoverflowcolumn.ecdb");

    RunPoint3dTest(point3dValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, BlobPerformance)
    {
    Utf8String stringValues[s_insertCount];
    GetRandomStrings(stringValues);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='BinaryProp' typeName='binary' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("binary_RegularColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "binaryinsertperformanceregularcolumn.ecdb");

    RunBlobTest(stringValues, "RegularColumn", true);
    GetECDb().CloseDb();

    Utf8CP schemaXmlWithSharedColumn = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='BinaryProp' typeName='binary' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    fileName.Sprintf("binary_OverflowColumn_opcount_%d_seed_%d.ecdb", s_insertCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "binaryinsertperformanceoverflowcolumn.ecdb");

    RunBlobTest(stringValues, "OverflowColumn", true);
    GetECDb().CloseDb();
    }

END_ECDBUNITTESTS_NAMESPACE
