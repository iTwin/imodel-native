/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECInstanceInserterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include <UnitTests/BackDoor/ECDb/Backdoor.h>
#include "PerformanceTestFixture.h"
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//*************************************************************************************************
// ECInstanceInserter vs ECSQL performance
//*************************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaCachePtr GenerateTestSchema()
{
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"Test\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"P\" isDomainClass = \"True\">"
        "    <ECProperty propertyName=\"bo\" typeName=\"boolean\" />"
        "    <ECProperty propertyName=\"bi\" typeName=\"binary\" />"
        "    <ECProperty propertyName=\"dt\" typeName=\"dateTime\" />"
        "    <ECProperty propertyName=\"d\" typeName=\"double\" />"
        "    <ECProperty propertyName=\"i\" typeName=\"int\" />"
        "    <ECProperty propertyName=\"l\" typeName=\"long\" />"
        "    <ECProperty propertyName=\"s\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"p2d\" typeName=\"Point2D\" />"
        "    <ECProperty propertyName=\"p3d\" typeName=\"Point3D\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"PStruct\" isDomainClass = \"False\" isStruct = \"True\">"
        "    <ECProperty propertyName=\"bo\" typeName=\"boolean\" />"
        "    <ECProperty propertyName=\"bi\" typeName=\"binary\" />"
        "    <ECProperty propertyName=\"dt\" typeName=\"dateTime\" />"
        "    <ECProperty propertyName=\"d\" typeName=\"double\" />"
        "    <ECProperty propertyName=\"i\" typeName=\"int\" />"
        "    <ECProperty propertyName=\"l\" typeName=\"long\" />"
        "    <ECProperty propertyName=\"s\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"p2d\" typeName=\"Point2D\" />"
        "    <ECProperty propertyName=\"p3d\" typeName=\"Point3D\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"Int\" isDomainClass = \"True\">"
        "    <ECProperty propertyName=\"i\" typeName=\"int\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"DateTime\" isDomainClass = \"True\">"
        "    <ECProperty propertyName=\"dt\" typeName=\"dateTime\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"String\" isDomainClass = \"True\">"
        "    <ECProperty propertyName=\"s\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"S\" >"
        "    <ECStructProperty propertyName=\"s\" typeName=\"PStruct\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"IntA\" >"
        "    <ECArrayProperty propertyName=\"a\" typeName=\"int\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"DateTimeA\" >"
        "    <ECArrayProperty propertyName=\"a\" typeName=\"dateTime\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"StringA\" >"
        "    <ECArrayProperty propertyName=\"a\" typeName=\"string\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"SA\" >"
        "    <ECArrayProperty propertyName=\"a\" typeName=\"PStruct\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
        "  </ECClass>"
        "</ECSchema>";

    return ECDbTestUtility::ReadECSchemaFromString(testSchemaXml);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SetupTestECDb(BeFileNameR ecdbPath, Utf8CP testECDbName)
{
    ECSchemaCachePtr schemaCache = GenerateTestSchema();
    if (schemaCache == nullptr)
        return ERROR;

    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create(testECDbName);
    if (ecdb.Schemas().ImportECSchemas(*schemaCache) != SUCCESS)
        return ERROR;

    ecdbPath = BeFileName(testProject.GetECDbPath());
    return SUCCESS;
}


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2014
//+===============+===============+===============+===============+===============+======
template<typename TImpl>
struct TestInserter
{
private:
    Utf8String m_testCaseName;

    TImpl* GetImpl() { return static_cast<TImpl*> (this); }

public:
    explicit TestInserter(Utf8CP testCaseName)
        : m_testCaseName(testCaseName)
    {}

    virtual ~TestInserter() {}

    bool Prepare(ECDbR ecdb, std::vector<ECN::ECClassCP> const& testClasses)
    {
        const auto success = GetImpl()->DoPrepare(ecdb, testClasses);
        if (!success)
            Finalize();

        return success;
    }

    bool InsertInstance(IECInstanceR testInstance)
    {
        const auto success = GetImpl()->DoInsertInstance(testInstance);
        if (!success)
            Finalize();

        return success;
    }

    void Finalize()
    {
        GetImpl()->DoFinalize();
    }

    Utf8CP GetTestCaseName() const { return m_testCaseName.c_str(); }
};

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceInserterTestInserter : TestInserter<ECInstanceInserterTestInserter>
{
private:
    std::map<ECN::ECClassCP, std::unique_ptr<ECInstanceInserter>>  m_cache;

    void Cleanup()
    {
        m_cache.clear();
    }

public:
    ECInstanceInserterTestInserter()
        : TestInserter("ECInstanceInserter")
    {
    }

    ~ECInstanceInserterTestInserter()
    {
        Cleanup();
    }

    bool DoPrepare(ECDbR ecdb, std::vector<ECN::ECClassCP> const& testClasses)
    {
        for (auto testClass : testClasses)
        {
            auto inserter = std::unique_ptr<ECInstanceInserter>(new ECInstanceInserter(ecdb, *testClass));
            if (!inserter->IsValid())
                return false;

            m_cache[testClass] = std::move(inserter);
        }

        return true;
    };

    bool DoInsertInstance(IECInstanceR testInstance) const
    {
        auto it = m_cache.find(&testInstance.GetClass());
        if (it == m_cache.end())
            return false;

        auto const& inserter = *it->second;

        ECInstanceKey key;
        return inserter.Insert(key, testInstance) == SUCCESS;
    };

    void DoFinalize()
    {
        Cleanup();
    }
};

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlTestInserter : TestInserter<ECSqlTestInserter>
{
private:
    std::map<ECN::ECClassCP, std::unique_ptr<ECSqlStatement>> m_cache;
    bmap <uint32_t, int> m_propertyIdToParameterMapping;

    bool Insert(ECSqlStatement& stmt, IECInstanceR testInstance) const
    {
        if (stmt.Reset() != ECSqlStatus::Success || stmt.ClearBindings() != ECSqlStatus::Success)
            return false;

        for (auto const& kvPair : m_propertyIdToParameterMapping)
        {
            const int parameterIndex = kvPair.second;
            ECValue v;
            Backdoor::ECObjects::ECValue::SetAllowsPointersIntoInstanceMemory(v, true);
            auto stat = testInstance.GetValue(v, kvPair.first);
            if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
                return false;

            if (v.IsNull())
            {
                if (ECSqlStatus::Success != stmt.BindNull(parameterIndex))
                    return false;

                continue;
            }

            auto ecsqlStat = ECSqlStatus::ProgrammerError;
            switch (v.GetPrimitiveType())
            {
            case PRIMITIVETYPE_Binary:
            {
                size_t size;
                auto ecBlob = v.GetBinary(size);
                ecsqlStat = stmt.BindBinary(parameterIndex, ecBlob, (int)size, DetermineMakeCopy(v));
                break;
            }

            case PRIMITIVETYPE_Boolean:
                ecsqlStat = stmt.BindBoolean(parameterIndex, v.GetBoolean());
                break;

            case PRIMITIVETYPE_DateTime:
            {
                bool hasMetadata = false;
                DateTime::Info metadata;
                const int64_t ceTicks = v.GetDateTimeTicks(hasMetadata, metadata);
                const uint64_t jdHns = DateTime::CommonEraTicksToJulianDay(ceTicks);

                DateTime::Info const* actualMetadata = hasMetadata ? &metadata : nullptr;
                ecsqlStat = Backdoor::ECDb::ECSqlStatement::BindDateTime(stmt, parameterIndex, jdHns, actualMetadata);
                break;
            }

            case PRIMITIVETYPE_Double:
                ecsqlStat = stmt.BindDouble(parameterIndex, v.GetDouble());
                break;

            case PRIMITIVETYPE_Integer:
                ecsqlStat = stmt.BindInt(parameterIndex, v.GetInteger());
                break;

            case PRIMITIVETYPE_Long:
                ecsqlStat = stmt.BindInt64(parameterIndex, v.GetLong());
                break;

            case PRIMITIVETYPE_Point2D:
                ecsqlStat = stmt.BindPoint2D(parameterIndex, v.GetPoint2D());
                break;

            case PRIMITIVETYPE_Point3D:
                ecsqlStat = stmt.BindPoint3D(parameterIndex, v.GetPoint3D());
                break;

            case PRIMITIVETYPE_String:
                ecsqlStat = stmt.BindText(parameterIndex, v.GetUtf8CP(), DetermineMakeCopy(v));
                break;

            default:
                break;
            }
            if (ecsqlStat != ECSqlStatus::Success)
                return false;
        }

        ECInstanceKey instanceKey;
        auto stepStat = stmt.Step(instanceKey);
        if (stepStat != ECSqlStepStatus::Done)
            return false;

        return true;
    }

    void Cleanup()
    {
        m_cache.clear();
    }

    static IECSqlBinder::MakeCopy DetermineMakeCopy(ECN::ECValueCR ecValue)
    {
        if (ecValue.IsString() && !ecValue.IsUtf8())
            return IECSqlBinder::MakeCopy::Yes;

        return Backdoor::ECObjects::ECValue::AllowsPointersIntoInstanceMemory(ecValue) ? IECSqlBinder::MakeCopy::No : IECSqlBinder::MakeCopy::Yes;
    }

public:
    ECSqlTestInserter()
        : TestInserter("ECSQL INSERT")
    {
    }

    virtual ~ECSqlTestInserter()
    {
        Cleanup();
    }


    bool DoPrepare(ECDbR ecdb, std::vector<ECN::ECClassCP> const& testClasses)
    {
        for (auto testClass : testClasses)
        {
            auto stmt = std::unique_ptr<ECSqlStatement>(new ECSqlStatement());
            auto enabler = testClass->GetDefaultStandaloneEnabler();
            int parameterIndex = 1;
            ECSqlInsertBuilder ecsql;
            ecsql.InsertInto(*testClass);
            for (auto prop : testClass->GetProperties(true))
            {
                if (!prop->GetIsPrimitive())
                {
                    return false;
                }

                Utf8String propName("[");
                propName.append(Utf8String(prop->GetName())).append("]");
                ecsql.AddValue(propName.c_str(), "?");

                uint32_t propIndex;
                auto stat = enabler->GetPropertyIndex(propIndex, prop->GetName().c_str());
                if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
                    return false;

                m_propertyIdToParameterMapping[propIndex] = parameterIndex;
                parameterIndex++;
            }

            auto stat = stmt->Prepare(ecdb, ecsql.ToString().c_str());
            if (stat != ECSqlStatus::Success)
                return false;

            m_cache[testClass] = std::move(stmt);
        }

        return true;
    }

    bool DoInsertInstance(IECInstanceR testInstance) const
    {
        auto it = m_cache.find(&testInstance.GetClass());
        if (it == m_cache.end())
            return false;

        auto& stmt = *it->second;
        return Insert(stmt, testInstance);
    }

    void DoFinalize()
    {
        Cleanup();
    }
};


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
template<typename TTestInserter>
void RunPerformanceComparison (bool& hasRun, double& insertTimingSecs, Utf8CP testSchemaName, Utf8CP testClassName, BeFileNameCR testFilePath,
                               int numberOfInstancesPerClass,
                               TestInserter<TTestInserter>& testInserter, StopWatch& logTimer)
    {
    hasRun = false;

    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (Db::OpenMode::ReadWrite)));

    std::vector<ECClassCP> testClasses;
    if (!Utf8String::IsNullOrEmpty (testClassName))
        {
        ECClassCP testClass = ecdb. Schemas ().GetECClass (testSchemaName, testClassName);
        ASSERT_TRUE (testClass != nullptr);
        testClasses.push_back (testClass);
        }
    else
        {
        ECSchemaCP schema = ecdb. Schemas ().GetECSchema (testSchemaName);
        ASSERT_TRUE (schema != nullptr);
        for (auto ecClass : schema->GetClasses ())
            {
            //filter out relationships to keep it simple and filter out non-domainclasses as ECPersistence doesn't support them
            if (ecClass->GetRelationshipClassCP () == nullptr && ecClass->GetIsDomainClass ())
                testClasses.push_back (ecClass);
            }
        }

    std::vector<IECInstancePtr> testDataset;
    for (auto ecClass : testClasses)
        {
        for (int i = 0; i < numberOfInstancesPerClass; i++)
            testDataset.push_back (ECDbTestProject::CreateArbitraryECInstance (*ecClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues));
        }

    if (!testInserter.Prepare (ecdb, testClasses))
        return;

    int failureCount = 0;
    //printf ("Attach to profiler for test case %s...\r\n", testInserter.GetTestCaseName ()); getchar ();
    StopWatch timer (true);
    logTimer.Start ();
    for (IECInstancePtr const& testInstance : testDataset)
        {
        if (!testInserter.InsertInstance (*testInstance))
            failureCount++;
        }
    timer.Stop ();
    logTimer.Stop ();
    testInserter.Finalize ();
    insertTimingSecs = timer.GetElapsedSeconds ();
    //printf ("Detach from profiler...\r\n"); getchar ();

    LOG.infov ("%s> Insertion (%d instances, %d per class, %d classes): %.4f secs.",
               testInserter.GetTestCaseName (),
               testDataset.size () - failureCount, numberOfInstancesPerClass, testClasses.size (), insertTimingSecs);
    hasRun = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
void RunPerformanceComparisonTest(Utf8CP testClassName, int numberOfInstancesPerClass,Utf8String TestDetails)
{
    Utf8CP testSchemaName = "Test";

    //Create test files
    BeFileName eciiTestFile;
    BeFileName ecsqlinsertTestFile;
    ASSERT_EQ(SUCCESS, SetupTestECDb(eciiTestFile, "ecinstanceinserterperformance.ecdb"));
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecsqlinsertTestFile, "ecsqlinsertperformance.ecdb"));

    //********** Run performance test with ECInstanceInserter
    StopWatch eciiInsertLogTimer("eciiInsertLogTimer",false);
    bool eciiHasRun = false;
    double eciiInsertTiming = -1.0;
    ECInstanceInserterTestInserter eciiTestInserter;
    //PerformanceTestingFrameWork performanceEciiTestInserter;
    Utf8String eciiTestInserterDetails(testClassName);
    Utf8PrintfString eciiInsertNumberOfClasses("%d", numberOfInstancesPerClass);
    RunPerformanceComparison(eciiHasRun, eciiInsertTiming, testSchemaName, testClassName, eciiTestFile, numberOfInstancesPerClass, eciiTestInserter, eciiInsertLogTimer);

    //********** Run performance test with ECSQL INSERT getting time zero have to take a look
    bool ecsqlHasRun = false;
    double ecsqlinsertInsertTiming = -1.0;
    ECSqlTestInserter ecsqlTestInserter;
    StopWatch ecsqlTestInserterLogTimer("ecsqlTestInserterLogTimer",false);
    //PerformanceTestingFrameWork performanceEcsqlTestInserter;
    Utf8String ecsqlTestInserterDetails(testClassName);
    Utf8PrintfString ecsqlTestInserterNumberOfClasses("%d", numberOfInstancesPerClass);
    RunPerformanceComparison(ecsqlHasRun, ecsqlinsertInsertTiming, testSchemaName, testClassName, ecsqlinsertTestFile, numberOfInstancesPerClass, ecsqlTestInserter, ecsqlTestInserterLogTimer);
    
    if (eciiHasRun)
    { 
        PERFORMANCELOG.infov("ECInstanceInserter Performance: Insertion: %.1f",
            eciiInsertTiming);
        LOGTODB(TEST_DETAILS, eciiInsertTiming, "ECInstanceInserter. Number or instances Per Class", numberOfInstancesPerClass);
    }
    if (ecsqlHasRun)
    {
        PERFORMANCELOG.infov("ECSQL INSERT Performance: Insertion: %.1f",
            ecsqlinsertInsertTiming);
        LOGTODB(TEST_DETAILS, ecsqlinsertInsertTiming, "ECSQL INSERT. Number or instances Per Class", numberOfInstancesPerClass);
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_ECSQLVersusECInstanceInserter, AllPropertyTypes)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,AllPropertyTypes";
    RunPerformanceComparisonTest(nullptr, 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, PrimitiveProperties)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,PrimitiveProperties";
    RunPerformanceComparisonTest("P", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, IntProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,IntProperty";
    RunPerformanceComparisonTest("Int", 1000, TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, DateTimeProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,DateTimeProperty";
    RunPerformanceComparisonTest("DateTime", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StringProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,StringProperty";
    RunPerformanceComparisonTest("String", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StructProperties)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,StructProperties";
    RunPerformanceComparisonTest("S", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, IntArrayProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,IntArrayProperty";
    RunPerformanceComparisonTest("IntA", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, DateTimeArrayProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,DateTimeArrayProperty";
    RunPerformanceComparisonTest("DateTimeA", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StringArrayProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,StringArrayProperty";
    RunPerformanceComparisonTest("StringA", 1000,TestDetails);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StructArrayProperty)
{
    Utf8String TestDetails = "PerformanceECSQLVersusECInstanceInserter,StructArrayProperty";
    RunPerformanceComparisonTest("SA", 1000,TestDetails);
}
END_ECDBUNITTESTS_NAMESPACE