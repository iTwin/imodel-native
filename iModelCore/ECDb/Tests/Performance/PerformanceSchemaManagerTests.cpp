/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceSchemaManagerTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       09/2016
//=======================================================================================
struct PerformanceSchemaManagerTests : ECDbTestFixture {};


//---------------------------------------------------------------------------------------
// @bsiclass                                      Krischan.Eberle       09/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSchemaManagerTests, ECClassIdLookup_AllClasses)
    {
    ECDbCR ecdb = SetupECDb("ecclassidlookup.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 10, ECDb::OpenParams(ECDb::OpenMode::Readonly));

    bvector<ECSchemaCP> schemas = ecdb.Schemas().GetECSchemas(true);
    bmap<Utf8String, bmap<Utf8String, ECClassId>> expectedClassIds;
    int classCount = 0;
    for (ECSchemaCP schema : schemas)
        {
        bmap<Utf8String, ECClassId>& classIdMap = expectedClassIds[schema->GetName()];
        for (ECClassCP ecClass : schema->GetClasses())
            {
            ASSERT_TRUE(ecClass->HasId());
            classIdMap[ecClass->GetName()] = ecClass->GetId();
            classCount++;
            }
        }

    auto runLookup = [] (StopWatch& timer, ECDbCR ecdb, bmap<Utf8String, bmap<Utf8String, ECClassId>> const& expectedClassIds, int repetitionCount)
        {
        timer.Start();
        for (int i = 0; i < repetitionCount; i++)
            {
            for (bpair<Utf8String, bmap<Utf8String, ECClassId>> const& kvPair : expectedClassIds)
                {
                Utf8StringCR schemaName = kvPair.first;

                for (bpair<Utf8String, ECClassId> const& innerKvPair : kvPair.second)
                    {
                    Utf8StringCR className = innerKvPair.first;
                    ECClassId expectedId = innerKvPair.second;
                    ECClassId actualId = ecdb.Schemas().GetECClassId(schemaName, className, ResolveSchema::BySchemaName);
                    ASSERT_EQ(expectedId.GetValue(), actualId.GetValue()) << "ECClass: " << schemaName.c_str() << "." << className.c_str();
                    }
                }
            }

        timer.Stop();
        };

    const int repetitionCount = 50;
    StopWatch timer(false);
    runLookup(timer, ecdb, expectedClassIds, repetitionCount);
    Utf8String logMessage;
    logMessage.Sprintf("ECClassId look-up by name for %d ECClasses in %d ECSchemas.", classCount, (int) schemas.size());
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), repetitionCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                      Krischan.Eberle       09/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSchemaManagerTests, ECClassIdLookup_SingleClass)
    {
    ECDbCR ecdb = SetupECDb("ecclassidlookup.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 10, ECDb::OpenParams(ECDb::OpenMode::Readonly));

    int classCount = -1;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ec.ECClassDef"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    classCount = stmt.GetValueInt(0);
    ASSERT_GT(classCount, 0);
    }

    Utf8CP testSchemaName = "ECSqlTest";
    Utf8CP testClassName = "PSA";
    ECClassId expectedClassId = ecdb.Schemas().GetECClassId(testSchemaName, testClassName);
    ASSERT_TRUE(expectedClassId.IsValid());

    const int repetitionCount = 100000;
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECClassId actualClassId = ecdb.Schemas().GetECClassId(testSchemaName, testClassName);
        ASSERT_EQ(expectedClassId.GetValue(), actualClassId.GetValue());
        }

    timer.Stop();
    Utf8String logMessage;
    logMessage.Sprintf("ECClassId look-up for a single ECClass in a file with %d ECClasses.", classCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), repetitionCount);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                      Krischan.Eberle       09/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSchemaManagerTests, ECClassIdLookupDuringECSqlPreparation_SingleClass)
    {
    ECDbCR ecdb = SetupECDb("ecclassidlookup.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 10, ECDb::OpenParams(ECDb::OpenMode::Readonly));

    int classCount = -1;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ec.ECClassDef"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    classCount = stmt.GetValueInt(0);
    ASSERT_GT(classCount, 0);
    }

    Utf8CP testSchemaName = "ECSqlTest";
    Utf8CP testClassName = "PSA";
    ECClassId expectedClassId = ecdb.Schemas().GetECClassId(testSchemaName, testClassName);
    ASSERT_TRUE(expectedClassId.IsValid());

    Utf8String ecsql;
    ecsql.Sprintf("SELECT NULL FROM %s.%s", testSchemaName, testClassName);

    const int repetitionCount = 10000;
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        }

    timer.Stop();
    Utf8String logMessage;
    logMessage.Sprintf("ECSQL preparation for a single ECClass with ECClassId cache in a file with %d ECClasses.", classCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), repetitionCount);
    }

//---------------------------------------------------------------------------------------
// This test compares two different SQLite SQL to retrieve the ECClassId from the ec_ tables
// @bsimethod                                      Krischan.Eberle       09/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSchemaManagerTests, GetECClassIdSqlScenarios)
    {
    ECDbCR ecdb = SetupECDb("ecclassidsqlscenarios.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaCP expectedSchema = ecdb.Schemas().GetECSchema("ECSqlTest", false);
    ASSERT_TRUE(expectedSchema != nullptr);
    Utf8CP testClassName1 = "ABounded";
    Utf8CP testClassName2 = "PSA";
    ECClassId expectedClassId1 = ecdb.Schemas().GetECClassId("ECSqlTest", testClassName1);
    ASSERT_TRUE(expectedClassId1.IsValid());
    ECClassId expectedClassId2 = ecdb.Schemas().GetECClassId("ECSqlTest", testClassName2);
    ASSERT_TRUE(expectedClassId2.IsValid());

    const int opCount = 100000;

    //Scenario 1: By schema and class name (via join)
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE s.Name=? AND c.Name=?"));
    StopWatch timer(true);
    for (int i = 0; i < opCount; i++)
        {
        const bool useTestClass1 = i % 2 == 0;
        Utf8CP className = useTestClass1 ? testClassName1 : testClassName2;
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "ECSqlTest", Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(2, className, Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        if (useTestClass1)
            ASSERT_EQ(expectedClassId1.GetValue(), stmt.GetValueUInt64(0));
        else
            ASSERT_EQ(expectedClassId2.GetValue(), stmt.GetValueUInt64(0));

        stmt.ClearBindings();
        stmt.Reset();
        }
    timer.Stop();
    stmt.Finalize();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "ECClassId by schema and class name (via join)", opCount);

    //Scenario 2: By schema id and no join
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Id FROM ec_Class WHERE SchemaId=? AND Name=?"));
    timer.Start();
    for (int i = 0; i < opCount; i++)
        {
        const bool useTestClass1 = i % 2 == 0;
        Utf8CP className = useTestClass1 ? testClassName1 : testClassName2;

        ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, expectedSchema->GetId()));
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(2, className, Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        if (useTestClass1)
            ASSERT_EQ(expectedClassId1.GetValue(), stmt.GetValueUInt64(0));
        else
            ASSERT_EQ(expectedClassId2.GetValue(), stmt.GetValueUInt64(0));

        stmt.ClearBindings();
        stmt.Reset();
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "ECClassId by schema id and class name (no join)", opCount);
    }

END_ECDBUNITTESTS_NAMESPACE
