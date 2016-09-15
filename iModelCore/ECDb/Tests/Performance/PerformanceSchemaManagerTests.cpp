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

    ecdb.ClearECDbCache();

    auto runLookup = [] (StopWatch& timer, ECDbCR ecdb, bmap<Utf8String, bmap<Utf8String, ECClassId>> const& expectedClassIds, int repetitionCount, bool useCache)
        {
        timer.Start();
        for (int i = 0; i < repetitionCount; i++)
            {
            for (bpair<Utf8String, bmap<Utf8String, ECClassId>> const& kvPair : expectedClassIds)
                {
                Utf8CP schemaName = kvPair.first.c_str();

                for (bpair<Utf8String, ECClassId> const& innerKvPair : kvPair.second)
                    {
                    Utf8CP className = innerKvPair.first.c_str();
                    ECClassId expectedId = innerKvPair.second;
                    ECClassId actualId = ecdb.Schemas().GetECClassId(schemaName, className, ResolveSchema::BySchemaName);
                    ASSERT_EQ(expectedId.GetValue(), actualId.GetValue()) << "ECClass: " << schemaName << "." << className;
                    if (!useCache)
                        ecdb.ClearECDbCache();
                    }
                }
            }

        timer.Stop();
        };

    const int repetitionCount = 50;
    StopWatch timer(false);
    runLookup(timer, ecdb, expectedClassIds, repetitionCount, true);
    Utf8String logMessage;
    logMessage.Sprintf("Cached ECClassId look-up by name for %d ECClasses in %d ECSchemas.", classCount, (int) schemas.size());
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), repetitionCount);

    ecdb.ClearECDbCache();
    runLookup(timer, ecdb, expectedClassIds, repetitionCount, false);
    logMessage.Sprintf("Uncached ECClassId look-up by name for %d ECClasses in %d ECSchemas.", classCount, (int) schemas.size());
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

    ecdb.ClearECDbCache();

    const int repetitionCount = 100000;
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECClassId actualClassId = ecdb.Schemas().GetECClassId(testSchemaName, testClassName);
        ASSERT_EQ(expectedClassId.GetValue(), actualClassId.GetValue());
        }

    timer.Stop();
    Utf8String logMessage;
    logMessage.Sprintf("Cached ECClassId look-up for a single ECClass in a file with %d ECClasses.", classCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), repetitionCount);

    ecdb.ClearECDbCache();
    timer.Start();
    for (int i = 0; i < repetitionCount; i++)
        {
        ECClassId actualClassId = ecdb.Schemas().GetECClassId(testSchemaName, testClassName);
        ASSERT_EQ(expectedClassId.GetValue(), actualClassId.GetValue());
        ecdb.ClearECDbCache();
        }

    timer.Stop();
    logMessage.Sprintf("Uncached ECClassId look-up for a single ECClass in a file with %d ECClasses.", classCount);
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

    ecdb.ClearECDbCache();

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

    ecdb.ClearECDbCache();
    timer.Start();
    for (int i = 0; i < repetitionCount; i++)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        stmt.Finalize();
        ecdb.ClearECDbCache();
        }

    timer.Stop();
    logMessage.Sprintf("ECSQL preparation for a single ECClass without ECClassId cache in a file with %d ECClasses", classCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), repetitionCount);
    }

END_ECDBUNITTESTS_NAMESPACE
