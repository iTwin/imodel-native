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
    static const int s_opCount = 50000;

    void GetInsertECSql(Utf8StringR insertECSql);

    void SetUpTestDb(Utf8String seedDbName, Utf8CP schemaXml, Utf8String destFileName);

    void RunIntInsertTest(int intValues[], Utf8String testDescription);
    void RunLongInsertTest(int64_t longValues[], Utf8String testDescription);
    void RunDoubleInsertTest(double doubleValues[], Utf8String testDescription);
    void RunBoolInsertTest(bool boolValues[], Utf8String testDescription);
    void RunStringInsertTest(Utf8String stringValues[], Utf8String testDescription);
    void RunPoint2dInsertTest(DPoint2d point2dValues[], Utf8String testDescription);
    void RunPoint3dInsertTest(std::vector<DPoint3d> point3dValues, Utf8String testDescription);
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
void PerformanceRegularVsOverFlowColumns::RunIntInsertTest(int intValues[], Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindInt(2, intValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunLongInsertTest(int64_t longValues[], Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindInt64(2, longValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunDoubleInsertTest(double doubleValues[], Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindDouble(2, doubleValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunBoolInsertTest(bool boolValues[], Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindBoolean(2, boolValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunStringInsertTest(Utf8String stringValues[], Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindText(2, stringValues[i].c_str(), IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunPoint2dInsertTest(DPoint2d point2dValues[], Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindPoint2d(2, point2dValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceRegularVsOverFlowColumns::RunPoint3dInsertTest(std::vector<DPoint3d> point3dValues, Utf8String testDescription)
    {
    Utf8String insertECSql;
    GetInsertECSql(insertECSql);
    ECSqlStatement statement;

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), insertECSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        statement.BindInt64(1, s_firstInstanceId + i);
        statement.BindPoint3d(2, point3dValues[i]);
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        }
    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescription.c_str(), s_opCount);
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, IntegerPerformance)
    {
    // get random int values between minimum and maximum integer
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    int intValues[s_opCount];
    for (int i = 0; i < s_opCount; i++)
        intValues[i] = dist(rd);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='IntProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("int_insert_RegularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "intinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("Integer_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunIntInsertTest(intValues, testDescription);
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

    fileName.Sprintf("int_insert_OverflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "intinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("Integer_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunIntInsertTest(intValues, testDescription);
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
    int64_t longValues[s_opCount];
    for (int i = 0; i < s_opCount; i++)
        longValues[i] = dist(rd);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='LongProp' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("long_insert_RegularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "longinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("Long_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunLongInsertTest(longValues, testDescription);
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

    fileName.Sprintf("long_insert_OverflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "longinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("Long_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunLongInsertTest(longValues, testDescription);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, DoublePerformance)
    {
    // get random int values between minimum and maximum Double
    std::random_device rd;
    std::uniform_real_distribution<double> dist(std::numeric_limits<double>().min(), std::numeric_limits<double>().max());
    double doubleValues[s_opCount];
    for (int i = 0; i < s_opCount; i++)
        doubleValues[i] = dist(rd);

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='DoubleProp' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("double_insert_regularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "doubleinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("Double_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunDoubleInsertTest(doubleValues, testDescription);
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

    fileName.Sprintf("double_insert_overflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "doubleinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("Double_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunDoubleInsertTest(doubleValues, testDescription);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, BoolPerformance)
    {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>().min(), std::numeric_limits<int>().max());
    bool boolValues[s_opCount];
    for (int i = 0; i < s_opCount; i++)
        boolValues[i] = dist(rd) % 2 != 0;

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='boolProp' typeName='boolean' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("bool_insert_RegularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "boolinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("Bool_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunBoolInsertTest(boolValues, testDescription);
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

    fileName.Sprintf("bool_insert_OverflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "boolinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("Bool_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunBoolInsertTest(boolValues, testDescription);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, TextPerformance)
    {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 61);
    Utf8String stringValues[s_opCount];

    static Utf8String charSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int j = 0; j < s_opCount; j++)
        {
        Utf8String string;
        int stringLength = dist(rd);
        string.resize(stringLength);
        for (int i = 0; i < stringLength; i++)
            string[i] = charSet[dist(rd)];

        stringValues[j] = string;
        }

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='stringProp' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("string_insert_regularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "stringinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("String_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunStringInsertTest(stringValues, testDescription);
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

    fileName.Sprintf("string_insert_OverflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "stringinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("String_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunStringInsertTest(stringValues, testDescription);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, Point2dPerformance)
    {
    std::random_device rd;
    std::uniform_real_distribution<double> dist(std::numeric_limits<double>().min(), std::numeric_limits<double>().max());
    DPoint2d point2dValues[s_opCount];
    for (int i = 0; i < s_opCount; i++)
        point2dValues[i] = DPoint2d::From(dist(rd), dist(rd));

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='Point2dProp' typeName='point2d' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("point2d_insert_RegularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "point2dinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("point2d_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunPoint2dInsertTest(point2dValues, testDescription);
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

    fileName.Sprintf("point2d_insert_OverflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "point2dinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("Point2d_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunPoint2dInsertTest(point2dValues, testDescription);
    GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceRegularVsOverFlowColumns, Point3dPerformance)
    {
    std::random_device rd;
    std::uniform_real_distribution<double> dist(std::numeric_limits<double>().min(), std::numeric_limits<double>().max());
    std::vector<DPoint3d> point3dValues;
    for (int i = 0; i < s_opCount; i++)
        point3dValues.push_back(DPoint3d::From(dist(rd), dist(rd), dist(rd)));

    Utf8CP schemaXml = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECProperty propertyName='Point3dProp' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>";

    Utf8String fileName;
    fileName.Sprintf("point3d_insert_RegularColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXml, "point3dinsertperformanceregularcolumn.ecdb");

    Utf8String testDescription;
    testDescription.Sprintf("point3d_InsertPerformance_RegularColumn [Initial count: %d]", s_initialInstanceCount);
    RunPoint3dInsertTest(point3dValues, testDescription);
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

    fileName.Sprintf("point3d_insert_OverflowColumn_opcount_%d_seed_%d.ecdb", s_opCount, DateTime::GetCurrentTimeUtc().GetDayOfYear());
    SetUpTestDb(fileName, schemaXmlWithSharedColumn, "point3dinsertperformanceoverflowcolumn.ecdb");

    testDescription.Sprintf("Point3d_InsertPerformance_OverflowColumn [Initial count: %d]", s_initialInstanceCount);
    RunPoint3dInsertTest(point3dValues, testDescription);
    GetECDb().CloseDb();
    }

END_ECDBUNITTESTS_NAMESPACE