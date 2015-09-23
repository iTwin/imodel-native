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
#include "../NonPublished/PublicApi/NonPublished/ECDb/ECDbTestProject.h"
#include "../BackDoor/PublicApi/BackDoor/ECDb/Backdoor.h"
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//struct Performance_ECSQLVersusECInstanceInserter : public ::testing::Test
//{
//public:
//    double m_eciiTime;
//    double m_ecsqlTime;
//
//    Performance_ECSQLVersusECInstanceInserter::Performance_ECSQLVersusECInstanceInserter() { m_eciiTime = m_ecsqlTime = 0.0; }
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

            ECSqlStatus ecsqlStat;
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
                ecsqlStat = stmt.GetBinder(parameterIndex).BindDateTime(jdHns, actualMetadata);
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
                ecsqlStat = ECSqlStatus::Error;
                break;
            }
            if (!ecsqlStat.IsSuccess())
                return false;
        }

        ECInstanceKey instanceKey;
        auto stepStat = stmt.Step(instanceKey);
        if (stepStat != BE_SQLITE_DONE)
            return false;

        return true;
    }

    void Cleanup() { m_cache.clear(); }

    static IECSqlBinder::MakeCopy DetermineMakeCopy(ECN::ECValueCR ecValue)
    {
        if (ecValue.IsString() && !ecValue.IsUtf8())
            return IECSqlBinder::MakeCopy::Yes;

        return Backdoor::ECObjects::ECValue::AllowsPointersIntoInstanceMemory(ecValue) ? IECSqlBinder::MakeCopy::No : IECSqlBinder::MakeCopy::Yes;
    }

public:
    ECSqlTestInserter() : TestInserter("ECSQL INSERT") {}

    virtual ~ECSqlTestInserter() { Cleanup(); }


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

    void DoFinalize() { Cleanup(); }
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
void RunPerformanceComparisonTest(Utf8CP testClassName, int numberOfInstancesPerClass, Utf8String testcaseName, Utf8String testName)
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
    Utf8String eciiTestInserterDetails(testClassName);
    Utf8PrintfString eciiInsertNumberOfClasses("%d", numberOfInstancesPerClass);
    RunPerformanceComparison(eciiHasRun, eciiInsertTiming, testSchemaName, testClassName, eciiTestFile, numberOfInstancesPerClass, eciiTestInserter, eciiInsertLogTimer);

    //********** Run performance test with ECSQL INSERT getting time zero have to take a look
    bool ecsqlHasRun = false;
    double ecsqlinsertInsertTiming = -1.0;
    ECSqlTestInserter ecsqlTestInserter;
    StopWatch ecsqlTestInserterLogTimer("ecsqlTestInserterLogTimer",false);
    Utf8String ecsqlTestInserterDetails(testClassName);
    Utf8PrintfString ecsqlTestInserterNumberOfClasses("%d", numberOfInstancesPerClass);
    RunPerformanceComparison(ecsqlHasRun, ecsqlinsertInsertTiming, testSchemaName, testClassName, ecsqlinsertTestFile, numberOfInstancesPerClass, ecsqlTestInserter, ecsqlTestInserterLogTimer);
    
    if (eciiHasRun)
    { 
        PERFORMANCELOG.infov("ECInstanceInserter Performance: Insertion: %.1f",
            eciiInsertTiming);
        LOGTODB(testcaseName, testName, eciiInsertTiming, "ECInstanceInserter Performance: Insertion: for", numberOfInstancesPerClass);
    }
    if (ecsqlHasRun)
    {
        PERFORMANCELOG.infov("ECSQL INSERT Performance: Insertion: %.1f",
            ecsqlinsertInsertTiming);
        LOGTODB(testcaseName, testName, ecsqlinsertInsertTiming, "ForECSQLInsert Performance: Insertion: for", numberOfInstancesPerClass);
    }
}

//}; // end of Test Fixture class

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_ECSQLVersusECInstanceInserter, AllPropertyTypes)
{
    RunPerformanceComparisonTest(nullptr, 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, PrimitiveProperties)
{
    RunPerformanceComparisonTest("P", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, IntProperty)
{
    RunPerformanceComparisonTest("Int", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, DateTimeProperty)
{
    RunPerformanceComparisonTest("DateTime", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StringProperty)
{
    RunPerformanceComparisonTest("String", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StructProperties)
{
    RunPerformanceComparisonTest("S", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, IntArrayProperty)
{
    RunPerformanceComparisonTest("IntA", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, DateTimeArrayProperty)
{
    RunPerformanceComparisonTest("DateTimeA", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StringArrayProperty)
{
    RunPerformanceComparisonTest("StringA", 1000, TEST_DETAILS);
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_ECSQLVersusECInstanceInserter, StructArrayProperty)
{
    RunPerformanceComparisonTest("SA", 1000, TEST_DETAILS);
}


//==================================================================================================================================================
//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
struct TestParamters
    {
    public:
        enum class CascadeMethod
            {
            ForiegnKey, Trigger, None
            };
    private:
        int m_noOfTargetClasses = 100;
        int m_noOfTargetTables = 100;
        int m_noOfColumnsInPrimaryTable = 5;
        int m_noOfColumnsInSecondaryTable = 4;
        int m_noOfInstances = 4;

        CascadeMethod  m_cascadeMethod = CascadeMethod::ForiegnKey;
        Utf8String m_toStr;
    public:
        TestParamters(int noOfTargetClasses, int noOfTargetTables, int noOfColumnsInPrimaryTable, int noOfColumnsInSecondaryTable, CascadeMethod cascadeMethod, int noOfInstances)
            :m_noOfTargetClasses(noOfTargetClasses), m_noOfTargetTables(noOfTargetTables), m_noOfColumnsInPrimaryTable(noOfColumnsInPrimaryTable), m_noOfColumnsInSecondaryTable(noOfColumnsInSecondaryTable), m_cascadeMethod(cascadeMethod), m_noOfInstances(noOfInstances)
            {

            m_toStr.Sprintf("Method = %s, NoOfTargetTables = %d, NoOfTargetClasses = %d, NoOfColumnsInPrimary = %d , NoOfColumnsInSecondary =%d , NoOfInstances = %d",
                m_cascadeMethod == CascadeMethod::ForiegnKey ? "ForiegnKey" : "Trigger",
                m_noOfTargetTables,
                m_noOfTargetClasses,
                m_noOfColumnsInPrimaryTable,
                m_noOfColumnsInSecondaryTable,
                m_noOfInstances
                );
            }
        TestParamters(TestParamters const& rhs)
            :m_noOfTargetClasses(rhs.m_noOfTargetClasses), m_noOfTargetTables(rhs.m_noOfTargetTables), m_noOfColumnsInPrimaryTable(rhs.m_noOfColumnsInPrimaryTable), m_noOfColumnsInSecondaryTable(rhs.m_noOfColumnsInSecondaryTable), m_cascadeMethod(rhs.m_cascadeMethod), m_toStr(rhs.m_toStr), m_noOfInstances(rhs.m_noOfInstances)
            {
            }
        TestParamters()
            :m_noOfTargetClasses(-1), m_noOfTargetTables(-1), m_noOfColumnsInPrimaryTable(-1), m_noOfColumnsInSecondaryTable(-1), m_cascadeMethod(CascadeMethod::None), m_noOfInstances(-1)
            {
            }
        Utf8String GetFileName() const
            {
            Utf8String str;
            str.Sprintf("Perf_%s_%d_%d_%d.db",
                m_cascadeMethod == CascadeMethod::ForiegnKey ? "ForiegnKey" : "Trigger",
                m_noOfTargetTables,
                m_noOfTargetClasses,
                m_noOfInstances
                );

            return str;
            }
        TestParamters& operator = (TestParamters const& rhs)
            {
            if (&rhs != this)
                {
                m_noOfTargetClasses = rhs.m_noOfTargetClasses;
                m_noOfTargetTables = rhs.m_noOfTargetTables;
                m_noOfColumnsInPrimaryTable = rhs.m_noOfColumnsInPrimaryTable;
                m_noOfColumnsInSecondaryTable = rhs.m_noOfColumnsInSecondaryTable;
                m_cascadeMethod = rhs.m_cascadeMethod;
                m_toStr = rhs.m_toStr;
                m_noOfInstances = rhs.m_noOfInstances;
                }
            return *this;
            }
        bool IsValid() const
            {
            return m_noOfTargetClasses > 0 && m_cascadeMethod != CascadeMethod::None && m_noOfColumnsInPrimaryTable > 0 && m_noOfColumnsInSecondaryTable > 0 && m_noOfTargetTables > 0;
            }
        int GetNumberOfTargetClasses() const { return m_noOfTargetClasses; }
        int GetNumberOfTargetTables() const { return m_noOfTargetTables; }
        int GetNumberOfColumnsInPrimaryTable() const { return m_noOfColumnsInPrimaryTable; }
        int GetNumberOfColumnsInSecondaryTable() const { return m_noOfColumnsInSecondaryTable; }
        CascadeMethod GetCascadeMethod() const { return m_cascadeMethod; }
        int GetNumberOfInstances() const { return m_noOfInstances; }
        Utf8CP ToString() const
            {
            return m_toStr.c_str();
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
struct TestResult
    {
    private:
        mutable StopWatch m_stopWatch;
        int m_noOfUnitDeletion;
        Utf8String m_name;
    public:
        TestResult(Utf8CP name = "")
            :m_noOfUnitDeletion(0), m_stopWatch(false), m_name(name)
            {
            }

        void Begin()
            {
            m_stopWatch.Start();
            }

        void End()
            {
            m_stopWatch.Stop();
            }
        void RecordUnitDeletion()
            {
            m_noOfUnitDeletion++;
            }
        void Reset(Utf8CP name)
            {
            m_stopWatch.Init(false);
            m_noOfUnitDeletion = 0;
            m_name = name;
            }
        Utf8String ToString(TestParamters const *param = nullptr) const
            {
            Utf8String str;
            double totalTimeInSec = m_stopWatch.GetElapsedSeconds();
            //double timeInMS = totalTimeInSec * 1000;
            //double timeInMSDeletion = timeInMS / m_noOfUnitDeletion;
            //str.Sprintf("PARAM  = %s\n", m_testParam.ToString());

            str.Sprintf("Time = %.4lf sec ",
                totalTimeInSec
                );

            if (!param)
                return str;

            Utf8String fullResult;
            fullResult.Sprintf("[Name: %s]\n PARAM  : %s \n RESULT : %s", m_name.c_str(), param->ToString(), str.c_str());
            return fullResult;
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
void SetupDeleteTest(DbR db, int64_t& globalInstanceCount, TestParamters const& param)
    {

    ECDbTestProject::Initialize();
    Utf8String dbPath = ECDbTestProject::BuildECDbPath(param.GetFileName().c_str());
    WString dbPathW;
    BeStringUtilities::Utf8ToWChar(dbPathW, dbPath.c_str());
    if (BeFileName::DoesPathExist(dbPathW.c_str()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(dbPathW.c_str());
        ASSERT_TRUE(fileDeleteStatus == BeFileNameStatus::Success);
        }

    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(dbPath.c_str())) << "Failed to create test db";
    ASSERT_EQ(true, param.IsValid()) << "Paramter provided to test was invalid";


    std::vector<Utf8String> primaryColumns;
    std::vector<Utf8String> secondaryColumns;
    std::map<Utf8String, std::set<int64_t>> secondaryTables;
    std::map<int64_t, Utf8String> secondaryTablesRev;
    for (int i = 1; i <= param.GetNumberOfColumnsInPrimaryTable(); i++)
        {
        primaryColumns.push_back(Utf8PrintfString("PCOL%03d", i));
        }
    for (int i = 1; i <= param.GetNumberOfColumnsInSecondaryTable(); i++)
        {
        secondaryColumns.push_back(Utf8PrintfString("SCOL%03d", i));
        }

    auto classPerTable = static_cast<int>(ceil((double)param.GetNumberOfTargetClasses() / param.GetNumberOfTargetTables()));
    ASSERT_TRUE(classPerTable > 0) << "Classes per table must be greater then zero";
    auto const seedClassId = 1;
    auto currentClassId = seedClassId;
    for (int i = 1; i <= param.GetNumberOfTargetTables(); i++)
        {
        Utf8String tableName;
        tableName.Sprintf("SecondaryTbl%03d", i);
        auto& classSet = secondaryTables[tableName];
        auto ubound = currentClassId + classPerTable;
        for (; currentClassId < ubound; currentClassId++)
            {
            classSet.insert(currentClassId);
            secondaryTablesRev[currentClassId] = tableName;
            }
        }

#define DELIMITER ", "
    auto joinStrFunc = [] (std::vector<Utf8String> const& strList )
        {
        Utf8String out;
        for (auto& str : strList)
            {
            out.append(str);
            if (&str != &strList.back())
                out.append(DELIMITER);
            }

        return out;
        };

    auto joinIntFunc = [] (std::set<int64_t> const& intList)
        {
        Utf8String out;
        for (auto& n : intList)
            {
            out.append(Utf8PrintfString("%lld", n).c_str());
            if (n != *intList.rbegin())
                out.append(DELIMITER);
            }

        return out;
        };
    auto repeatFunc = [] (Utf8CP token, size_t times)
        {
        Utf8String out;
        for (size_t i = 0; i < times; i++)
            {
            out.append(token);
            if (i != (times - 1))
                out.append(DELIMITER);
            }

        return out;
        };
    auto bindData = [] (Statement& stmt, std::vector<Utf8String> columns, int startingIndex)
        {
        for (size_t  i = 0; i < columns.size(); i++)
            {
            stmt.BindText(startingIndex++, "(asdfa987a9idsf#@#$SAdfasdfkajlaksjdf", Statement::MakeCopy::No);
            }
        };
    Utf8String createTableSql_Primary = "CREATE TABLE PrimaryTbl (Id INTEGER PRIMARY KEY, ClassId INTEGER NOT NULL";
    if (!primaryColumns.empty())
        createTableSql_Primary.append(",").append(joinStrFunc(primaryColumns));

    createTableSql_Primary.append(");");

    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.ExecuteSql(createTableSql_Primary.c_str())) << "FAILED " << createTableSql_Primary.c_str();

    auto dataColumns = joinStrFunc(secondaryColumns);
    for (auto const& secondaryPair : secondaryTables)
        {
        Utf8String createTableSql_Secondary = "CREATE TABLE ";
        createTableSql_Secondary.append(secondaryPair.first);
        createTableSql_Secondary.append("(");
        createTableSql_Secondary.append("Id INTEGER PRIMARY KEY ");
        if (param.GetCascadeMethod() == TestParamters::CascadeMethod::ForiegnKey)
            {
            createTableSql_Secondary.append("REFERENCES PrimaryTbl(Id) ON DELETE CASCADE");
            }

        createTableSql_Secondary.append(", ClassId INTEGER NOT NULL");
        if (!secondaryColumns.empty())
            createTableSql_Secondary.append(",").append(dataColumns);

        createTableSql_Secondary.append(");");
        ASSERT_EQ(DbResult::BE_SQLITE_OK, db.ExecuteSql(createTableSql_Secondary.c_str())) << "FAILED " << createTableSql_Secondary.c_str();

        if (param.GetCascadeMethod() == TestParamters::CascadeMethod::Trigger)
            {
            Utf8String trigger = "CREATE TRIGGER OnDelete";
            trigger.append(secondaryPair.first);
            trigger.append(" AFTER DELETE ON PrimaryTbl WHEN OLD.ClassId  IN (").append(joinIntFunc(secondaryPair.second)).append(")");
            trigger.append("BEGIN ");
            trigger.append(" DELETE FROM ").append(secondaryPair.first).append(" WHERE OLD.Id = Id; ");
            trigger.append(" END");
            ASSERT_EQ(DbResult::BE_SQLITE_OK, db.ExecuteSql(trigger.c_str())) << "FAILED " << trigger.c_str();
            }
        }

    Utf8String primaryInsert = "INSERT INTO PrimaryTbl(Id, ClassId";
    if (!primaryColumns.empty())
        primaryInsert.append(",").append(joinStrFunc(primaryColumns));

    primaryInsert.append(") VALUES (?, ?");
    if (!primaryColumns.empty())
        primaryInsert.append(",").append(repeatFunc("?", primaryColumns.size()));

    primaryInsert.append(");");


    Utf8String secondaryInsertTemplate = "INSERT INTO %s(Id, ClassId";
    if (!secondaryColumns.empty())
        secondaryInsertTemplate.append(",").append(joinStrFunc(secondaryColumns));

    secondaryInsertTemplate.append(") VALUES (?, ?");
    if (!secondaryColumns.empty())
        secondaryInsertTemplate.append(",").append(repeatFunc("?", secondaryColumns.size()));

    secondaryInsertTemplate.append(");");

    auto const noOfInstancesPerTable = param.GetNumberOfInstances() / param.GetNumberOfTargetTables();
    int64_t globalInstanceId = 0;
    Statement primaryInsertStmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, primaryInsertStmt.Prepare(db, primaryInsert.c_str()));
    for (auto const& secondaryPair : secondaryTables)
        {
        auto& table = secondaryPair.first;
        auto& classIds = secondaryPair.second;
        Statement secondaryInsertStmt;
        ASSERT_EQ(DbResult::BE_SQLITE_OK, secondaryInsertStmt.Prepare(db, SqlPrintfString(secondaryInsertTemplate.c_str(), table.c_str()).GetUtf8CP()));
        auto const noOfInstancesPerClass = noOfInstancesPerTable / classIds.size();
        for (auto cid : classIds)
            {
            for (size_t i = 0; i < noOfInstancesPerClass; i++)
                {
                globalInstanceId++;
                primaryInsertStmt.ClearBindings();
                primaryInsertStmt.Reset();
                primaryInsertStmt.BindInt64(1, globalInstanceId);
                primaryInsertStmt.BindInt64(2, cid);
                bindData(primaryInsertStmt, primaryColumns, 3);
                ASSERT_EQ(DbResult::BE_SQLITE_DONE, primaryInsertStmt.Step());

                secondaryInsertStmt.ClearBindings();
                secondaryInsertStmt.Reset();
                secondaryInsertStmt.BindInt64(1, globalInstanceId);
                secondaryInsertStmt.BindInt64(2, cid);
                bindData(primaryInsertStmt, secondaryColumns, 3);
                ASSERT_EQ(DbResult::BE_SQLITE_DONE, secondaryInsertStmt.Step());
                }
            }
        }

    globalInstanceCount = globalInstanceId;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
void ExecuteDeleteTest(TestParamters const& param, TestResult& r1, TestResult& r2)
    {
    int64_t globalInstanceCount = 0;
    Db db;
    SetupDeleteTest(db, globalInstanceCount, param);
    r1.Reset("*** Truncate Table ***");
    r2.Reset("*** Delete By Id one record at a time ***");

    Savepoint test1(db, "test1");
    Statement deleteAllStmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, deleteAllStmt.Prepare(db, "DELETE FROM PrimaryTbl"));
    r1.Begin();
    r1.RecordUnitDeletion();
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, deleteAllStmt.Step());
    r1.End();
    test1.Cancel();


    Savepoint test2(db, "test2");
    r2.Begin();
    Statement deleteSingleStmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, deleteSingleStmt.Prepare(db, "DELETE FROM PrimaryTbl WHERE Id = ?"));
    for (int64_t i = 1; i < globalInstanceCount; i++)
        {
        deleteSingleStmt.Reset();
        deleteSingleStmt.ClearBindings();
        deleteSingleStmt.BindInt64(1, i);
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, deleteSingleStmt.Step());
        r2.RecordUnitDeletion();
        }

    r2.End();
    test2.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_TriggerVsCascadeDelete, V1)
    {
    Db db;
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);
    
    
    TestResult r1, r2;
    int noOfClasses = 200;
    int noOfColumnInPrimary = 5;
    int noOfColumnInSecondary = 5;
    int noOfInstances = 50000;

    LOG.info("*** Number Of Classes = 500 ***");
    LOG.info("*** With 10 Tables ***");
    TestParamters fkParam  = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);   
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    TestParamters trParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 50 Tables ***");
    fkParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());


    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 200 Tables ***");
    fkParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_TriggerVsCascadeDelete, V2)
    {
    Db db;
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);


    TestResult r1, r2;
    int noOfClasses = 500;
    int noOfColumnInPrimary = 5;
    int noOfColumnInSecondary = 5;
    int noOfInstances = 50000;

    LOG.info("*** Number Of Classes = 500 ***");
    LOG.info("*** With 10 Tables ***");
    TestParamters fkParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    TestParamters trParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 50 Tables ***");
    fkParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());


    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 200 Tables ***");
    fkParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_TriggerVsCascadeDelete, V3)
    {
    Db db;
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);


    TestResult r1, r2;
    int noOfClasses = 700;
    int noOfColumnInPrimary = 5;
    int noOfColumnInSecondary = 5;
    int noOfInstances = 50000;

    LOG.info("*** Number Of Classes = 700 ***");
    LOG.info("*** With 10 Tables ***");
    TestParamters fkParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    TestParamters trParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 50 Tables ***");
    fkParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());


    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 200 Tables ***");
    fkParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_TriggerVsCascadeDelete, V4)
    {
    Db db;
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);


    TestResult r1, r2;
    int noOfClasses = 1000;
    int noOfColumnInPrimary = 5;
    int noOfColumnInSecondary = 5;
    int noOfInstances = 50000;

    LOG.info("*** Number Of Classes = 500 ***");
    LOG.info("*** With 10 Tables ***");
    TestParamters fkParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    TestParamters trParam = TestParamters(noOfClasses, 10, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 50 Tables ***");
    fkParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 50, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());


    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 100, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 100 Tables ***");
    fkParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 150, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());

    LOG.info("*** With 200 Tables ***");
    fkParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::ForiegnKey, noOfInstances);
    ExecuteDeleteTest(fkParam, r1, r2);
    LOG.infov("FK> Truncate: %s", r1.ToString().c_str());
    LOG.infov("FK> OnByOn  : %s", r2.ToString().c_str());

    trParam = TestParamters(noOfClasses, 200, noOfColumnInPrimary, noOfColumnInSecondary, TestParamters::CascadeMethod::Trigger, noOfInstances);
    ExecuteDeleteTest(trParam, r1, r2);
    LOG.infov("TR> Truncate: %s", r1.ToString().c_str());
    LOG.infov("TR> OnByOn  : %s", r2.ToString().c_str());
    }


END_ECDBUNITTESTS_NAMESPACE