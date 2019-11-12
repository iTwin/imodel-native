/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/BeSQLite.h>
#include "PerformanceTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceECSQLVersusECInstanceInserterTests : ECDbTestFixture
    {
    protected:
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

                bool Prepare(ECDbCR ecdb, std::vector<ECN::ECClassCP> const& testClasses)
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
                    {}

                ~ECInstanceInserterTestInserter()
                    {
                    Cleanup();
                    }

                bool DoPrepare(ECDbCR ecdb, std::vector<ECN::ECClassCP> const& testClasses)
                    {
                    for (auto testClass : testClasses)
                        {
                        std::unique_ptr<ECInstanceInserter> inserter(new ECInstanceInserter(ecdb, *testClass, nullptr));
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
                    return inserter.Insert(key, testInstance) == BE_SQLITE_OK;
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
                        BackDoor::ECObjects::ECValue::SetAllowsPointersIntoInstanceMemory(v, true);
                        auto stat = testInstance.GetValue(v, kvPair.first);
                        if (stat != ECObjectsStatus::Success)
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
                                ecsqlStat = stmt.BindBlob(parameterIndex, ecBlob, (int) size, DetermineMakeCopy(v));
                                break;
                                }

                                case PRIMITIVETYPE_Boolean:
                                    ecsqlStat = stmt.BindBoolean(parameterIndex, v.GetBoolean());
                                    break;

                                case PRIMITIVETYPE_DateTime:
                                {
                                DateTime::Info metadata;
                                const int64_t ceTicks = v.GetDateTimeTicks(metadata);
                                const uint64_t jdHns = DateTime::CommonEraMillisecondsToJulianDay(ceTicks / 10000);
                                ecsqlStat = stmt.GetBinder(parameterIndex).BindDateTime(jdHns, metadata);
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

                                case PRIMITIVETYPE_Point2d:
                                    ecsqlStat = stmt.BindPoint2d(parameterIndex, v.GetPoint2d());
                                    break;

                                case PRIMITIVETYPE_Point3d:
                                    ecsqlStat = stmt.BindPoint3d(parameterIndex, v.GetPoint3d());
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

                    return BackDoor::ECObjects::ECValue::AllowsPointersIntoInstanceMemory(ecValue) ? IECSqlBinder::MakeCopy::No : IECSqlBinder::MakeCopy::Yes;
                    }

            public:
                ECSqlTestInserter() : TestInserter("ECSQL INSERT") {}

                virtual ~ECSqlTestInserter() { Cleanup(); }

                bool DoPrepare(ECDbCR ecdb, std::vector<ECN::ECClassCP> const& testClasses)
                    {
                    for (auto testClass : testClasses)
                        {
                        auto stmt = std::unique_ptr<ECSqlStatement>(new ECSqlStatement());
                        auto enabler = testClass->GetDefaultStandaloneEnabler();
                        int parameterIndex = 1;
                        Utf8String ecsql("INSERT INTO ");
                        ecsql.append(testClass->GetECSqlName());
                        Utf8String ecsqlValuesClause(") VALUES(");
                        bool isFirstProp = true;
                        for (ECPropertyCP prop : testClass->GetProperties(true))
                            {
                            if (!prop->GetIsPrimitive())
                                return false;

                            if (!isFirstProp)
                                {
                                ecsql.append(",");
                                ecsqlValuesClause.append(",");
                                }

                            ecsql.append("[").append(prop->GetName()).append("]");
                            ecsqlValuesClause.append("?");

                            uint32_t propIndex;
                            auto stat = enabler->GetPropertyIndex(propIndex, prop->GetName().c_str());
                            if (stat != ECObjectsStatus::Success)
                                return false;

                            m_propertyIdToParameterMapping[propIndex] = parameterIndex;
                            parameterIndex++;
                            isFirstProp = false;
                            }

                        ecsql.append(ecsqlValuesClause).append(")");
                        auto stat = stmt->Prepare(ecdb, ecsql.c_str());
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
        void RunPerformanceComparison(bool& hasRun, double& insertTimingSecs, Utf8CP testSchemaName, Utf8CP testClassName, ECDbCR ecdb,
                                      int numberOfInstancesPerClass,
                                      TestInserter<TTestInserter>& testInserter, StopWatch& logTimer)
            {
            hasRun = false;

            std::vector<ECClassCP> testClasses;
            if (!Utf8String::IsNullOrEmpty(testClassName))
                {
                ECClassCP testClass = ecdb.Schemas().GetClass(testSchemaName, testClassName);
                ASSERT_TRUE(testClass != nullptr);
                testClasses.push_back(testClass);
                }
            else
                {
                ECSchemaCP schema = ecdb.Schemas().GetSchema(testSchemaName);
                ASSERT_TRUE(schema != nullptr);
                for (auto ecClass : schema->GetClasses())
                    {
                    //filter out relationships to keep it simple and filter out non-domainclasses as ECPersistence doesn't support them
                    if (ecClass->IsEntityClass() && ECClassModifier::Abstract != ecClass->GetClassModifier())
                        testClasses.push_back(ecClass);
                    }
                }

            std::vector<IECInstancePtr> testDataset;
            for (ECClassCP ecClass : testClasses)
                {
                for (int i = 0; i < numberOfInstancesPerClass; i++)
                    {
                    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
                    ECInstancePopulator::Populate(*instance);
                    testDataset.push_back(instance);
                    }
                }

            if (!testInserter.Prepare(ecdb, testClasses))
                return;

            int failureCount = 0;
            //printf ("Attach to profiler for test case %s...\r\n", testInserter.GetTestCaseName ()); getchar ();
            StopWatch timer(true);
            logTimer.Start();
            for (IECInstancePtr const& testInstance : testDataset)
                {
                if (!testInserter.InsertInstance(*testInstance))
                    failureCount++;
                }
            timer.Stop();
            logTimer.Stop();
            testInserter.Finalize();
            insertTimingSecs = timer.GetElapsedSeconds();
            //printf ("Detach from profiler...\r\n"); getchar ();

            LOG.infov("%s> Insertion (%d instances, %d per class, %d classes): %.4f secs.",
                      testInserter.GetTestCaseName(),
                      testDataset.size() - failureCount, numberOfInstancesPerClass, testClasses.size(), insertTimingSecs);
            hasRun = true;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  07/14
        //+---------------+---------------+---------------+---------------+---------------+------
        void RunPerformanceComparisonTest(Utf8CP testClassName, int numberOfInstancesPerClass)
            {
            Utf8CP testSchemaName = "Test";

            //********** Run performance test with ECInstanceInserter
            ASSERT_EQ(SUCCESS, SetupECDb("ecinstanceinserterperformance.ecdb", SchemaItem(GetTestSchemaXml())));

            StopWatch eciiInsertLogTimer("eciiInsertLogTimer", false);
            bool eciiHasRun = false;
            double eciiInsertTiming = -1.0;
            ECInstanceInserterTestInserter eciiTestInserter;
            Utf8String eciiTestInserterDetails(testClassName);
            Utf8PrintfString eciiInsertNumberOfClasses("%d", numberOfInstancesPerClass);
            RunPerformanceComparison(eciiHasRun, eciiInsertTiming, testSchemaName, testClassName, m_ecdb, numberOfInstancesPerClass, eciiTestInserter, eciiInsertLogTimer);
            m_ecdb.CloseDb();

            //********** Run performance test with ECSQL INSERT getting time zero have to take a look
            ASSERT_EQ(SUCCESS, SetupECDb("ecsqlinsertperformance.ecdb", SchemaItem(GetTestSchemaXml())));

            bool ecsqlHasRun = false;
            double ecsqlinsertInsertTiming = -1.0;
            ECSqlTestInserter ecsqlTestInserter;
            StopWatch ecsqlTestInserterLogTimer("ecsqlTestInserterLogTimer", false);
            Utf8String ecsqlTestInserterDetails(testClassName);
            Utf8PrintfString ecsqlTestInserterNumberOfClasses("%d", numberOfInstancesPerClass);
            RunPerformanceComparison(ecsqlHasRun, ecsqlinsertInsertTiming, testSchemaName, testClassName, m_ecdb, numberOfInstancesPerClass, ecsqlTestInserter, ecsqlTestInserterLogTimer);

            if (eciiHasRun)
                {
                PERFORMANCELOG.infov("ECInstanceInserter Performance: Insertion: %.1f",
                                     eciiInsertTiming);
                LOGTODB(TEST_DETAILS, eciiInsertTiming, numberOfInstancesPerClass, "ECInstanceInserter");
                }
            if (ecsqlHasRun)
                {
                PERFORMANCELOG.infov("ECSQL INSERT Performance: Insertion: %.1f",
                                     ecsqlinsertInsertTiming);
                LOGTODB(TEST_DETAILS, ecsqlinsertInsertTiming, numberOfInstancesPerClass, "ECSQL INSERT");
                }
            }


        static Utf8CP GetTestSchemaXml()
            {
            return "<ECSchema schemaName=\"Test\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                "  <ECEntityClass typeName=\"P\">"
                "    <ECProperty propertyName=\"bo\" typeName=\"boolean\" />"
                "    <ECProperty propertyName=\"bi\" typeName=\"binary\" />"
                "    <ECProperty propertyName=\"dt\" typeName=\"dateTime\" />"
                "    <ECProperty propertyName=\"d\" typeName=\"double\" />"
                "    <ECProperty propertyName=\"i\" typeName=\"int\" />"
                "    <ECProperty propertyName=\"l\" typeName=\"long\" />"
                "    <ECProperty propertyName=\"s\" typeName=\"string\" />"
                "    <ECProperty propertyName=\"p2d\" typeName=\"Point2D\" />"
                "    <ECProperty propertyName=\"p3d\" typeName=\"Point3D\" />"
                "  </ECEntityClass>"
                "  <ECStructClass typeName=\"PStruct\">"
                "    <ECProperty propertyName=\"bo\" typeName=\"boolean\" />"
                "    <ECProperty propertyName=\"bi\" typeName=\"binary\" />"
                "    <ECProperty propertyName=\"dt\" typeName=\"dateTime\" />"
                "    <ECProperty propertyName=\"d\" typeName=\"double\" />"
                "    <ECProperty propertyName=\"i\" typeName=\"int\" />"
                "    <ECProperty propertyName=\"l\" typeName=\"long\" />"
                "    <ECProperty propertyName=\"s\" typeName=\"string\" />"
                "    <ECProperty propertyName=\"p2d\" typeName=\"Point2D\" />"
                "    <ECProperty propertyName=\"p3d\" typeName=\"Point3D\" />"
                "  </ECStructClass>"
                "  <ECEntityClass typeName=\"Int\" >"
                "    <ECProperty propertyName=\"i\" typeName=\"int\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"DateTime\" >"
                "    <ECProperty propertyName=\"dt\" typeName=\"dateTime\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"String\" >"
                "    <ECProperty propertyName=\"s\" typeName=\"string\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"S\" >"
                "    <ECStructProperty propertyName=\"s\" typeName=\"PStruct\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"IntA\" >"
                "    <ECArrayProperty propertyName=\"a\" typeName=\"int\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"DateTimeA\" >"
                "    <ECArrayProperty propertyName=\"a\" typeName=\"dateTime\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"StringA\" >"
                "    <ECArrayProperty propertyName=\"a\" typeName=\"string\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName=\"SA\" >"
                "    <ECStructArrayProperty propertyName=\"a\" typeName=\"PStruct\" maxOccurs=\"unbounded\" minOccurs=\"0\" />"
                "  </ECEntityClass>"
                "</ECSchema>";
            }
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, AllPropertyTypes)
    {
    RunPerformanceComparisonTest(nullptr, 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, PrimitiveProperties)
    {
    RunPerformanceComparisonTest("P", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, IntProperty)
    {
    RunPerformanceComparisonTest("Int", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, DateTimeProperty)
    {
    RunPerformanceComparisonTest("DateTime", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, StringProperty)
    {
    RunPerformanceComparisonTest("String", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, StructProperties)
    {
    RunPerformanceComparisonTest("S", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, IntArrayProperty)
    {
    RunPerformanceComparisonTest("IntA", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, DateTimeArrayProperty)
    {
    RunPerformanceComparisonTest("DateTimeA", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, StringArrayProperty)
    {
    RunPerformanceComparisonTest("StringA", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSQLVersusECInstanceInserterTests, StructArrayProperty)
    {
    RunPerformanceComparisonTest("SA", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_TiggerVsSQL, BulkTest10000)
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    Db db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(":memory:"));
    Db dbv;
    ASSERT_EQ(BE_SQLITE_OK, dbv.CreateNewDb(":memory:"));
    std::vector<Utf8CP> sqlscripts
        =
        {
        " DROP TABLE IF EXISTS Foo_P;",

        " DROP TABLE IF EXISTS Foo_S;",

        " DROP VIEW  IF EXISTS Foo_V;",

        " CREATE TABLE Foo_P (ECInstanceId INTEGER PRIMARY KEY, ECClassId INTEGER, A1, A2, A3, A4, A5);",

        " CREATE TABLE Foo_S (ECInstanceId INTEGER PRIMARY KEY REFERENCES Foo_P(ECInstanceId) ON DELETE CASCADE, B1, B2, B3, B4, B5);",

        " CREATE VIEW Foo_V AS"
        "   SELECT "
        "  Foo_P.ECInstanceId, "
        "  Foo_P.ECClassId, "
        "  Foo_P.A1,"
        "  Foo_P.A2,"
        "  Foo_P.A3,"
        "  Foo_P.A4,"
        "  Foo_P.A5,"
        "  Foo_S.B1,"
        "  Foo_S.B2,"
        "  Foo_S.B3,"
        "  Foo_S.B4,"
        "  Foo_S.B5 "
        "  FROM Foo_P "
        "       INNER JOIN Foo_S ON Foo_P.ECInstanceId = Foo_S.ECInstanceId;",

        " CREATE TRIGGER Foo_Insert "
        " INSTEAD OF INSERT"
        " ON Foo_V"
        " BEGIN"
        "      INSERT INTO Foo_P(ECInstanceId, ECClassId, A1, A2, A3, A4, A5) VALUES (NEW.ECInstanceId, NEW.ECClassId, NEW.A1, NEW.A2, NEW.A3, NEW.A4, NEW.A5);"
        "      INSERT INTO Foo_S(ECInstanceId, B1, B2, B3, B4, B5) VALUES (NEW.ECInstanceId, NEW.B1, NEW.B2, NEW.B3, NEW.B4, NEW.B5);  "
        " END;",

        " CREATE TRIGGER Foo_Update_P"
        " INSTEAD OF UPDATE OF A1, A2, A3, A4, A5"
        " ON Foo_V"
        " BEGIN"
        "      UPDATE Foo_P SET  A1 = NEW.A1, A2 = NEW.A2, A3 = NEW.A3, A4 = NEW.A4, A5 = NEW.A5  WHERE ECInstanceId = OLD.ECInstanceId;"
        " END;",

        " CREATE TRIGGER Foo_Update_S"
        " INSTEAD OF UPDATE OF B1, B2, B3, B4, B5"
        " ON Foo_V"
        " BEGIN"
        "      UPDATE Foo_S SET  B1 = NEW.B1, B2 = NEW.B2, B3 = NEW.B3, B4 = NEW.B4, B5 = NEW.B5  WHERE ECInstanceId = OLD.ECInstanceId;"
        " END;",

        " CREATE TRIGGER Foo_Delete"
        " INSTEAD OF DELETE"
        " ON Foo_V"
        " BEGIN"
        "      DELETE FROM Foo_P WHERE ECInstanceId = OLD.ECInstanceId;"
        " END;"};

    for (auto statement : sqlscripts)
        {
        ASSERT_EQ(db.ExecuteSql(statement), BE_SQLITE_OK);
        ASSERT_EQ(dbv.ExecuteSql(statement), BE_SQLITE_OK);
        }
    enum class stype
        {
        foo_p_insert = 101,
        foo_s_insert = 102,
        foo_v_insert = 10001,
        foo_p_update = 201,
        foo_s_update = 202,
        foo_v_update = 20001,
        foo_p_delete = 301,
        foo_s_delete = 302,
        foo_v_delete = 30001,
        foo_p_select = 401,
        foo_s_select = 402,
        foo_v_select = 40001,
        };


    std::map<stype, Utf8CP> cruds;
    cruds[stype::foo_p_insert] = "INSERT INTO Foo_P (ECInstanceId, ECClassId, A1, A2, A3, A4, A5) VALUES (?, ?, ?, ?, ?, ?, ?);";
    cruds[stype::foo_s_insert] = "INSERT INTO Foo_S (ECInstanceId, B1, B2, B3, B4, B5) VALUES (?, ?, ?, ?, ?, ?);";
    cruds[stype::foo_v_insert] = "INSERT INTO Foo_V (ECInstanceId,ECClassId, A1, A2, A3, A4, A5, B1, B2, B3, B4, B5) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    cruds[stype::foo_p_update] = "UPDATE Foo_P  SET A1 = ?, A2 = ?, A3 = ?, A4 = ?, A5 = ? WHERE ECInstanceId = ?;";
    cruds[stype::foo_s_update] = "UPDATE Foo_S  SET B1 = ?, B2 = ?, B3 = ?, B4 = ?, B5 = ? WHERE ECInstanceId = ?;";
    cruds[stype::foo_v_update] = "UPDATE Foo_V SET A1 = ?, A2 = ?, A3 = ?, A4 = ?, A5 = ?, B1 = ?, B2 = ?, B3 = ?, B4 = ?, B5 = ? WHERE ECInstanceId = ?;";
    cruds[stype::foo_p_delete] = "DELETE FROM Foo_P WHERE ECInstanceId = ?;";
    cruds[stype::foo_s_delete] = cruds[stype::foo_p_delete];
    cruds[stype::foo_v_delete] = "DELETE FROM Foo_V WHERE ECInstanceId = ?;";
    cruds[stype::foo_p_select] = "SELECT ECInstanceId, ECClassId, A1, A2, A3, A4, A5 FROM Foo_P WHERE ECInstanceId = ?;";
    cruds[stype::foo_s_select] = "SELECT Foo_P.ECInstanceId, ECClassId, A1, A2, A3, A4, A5, B1, B2, B3, B4 ,B5 FROM Foo_P INNER JOIN Foo_S ON Foo_P.ECInstanceId = Foo_S.ECInstanceId WHERE Foo_P.ECInstanceId = ?;";
    cruds[stype::foo_v_select] = "SELECT ECInstanceId, ECClassId, A1, A2, A3, A4, A5, B1, B2, B3, B4 ,B5 FROM Foo_V WHERE ECInstanceId = ?;";

    std::map<stype, Statement> stmts;
    for (auto& e : cruds)
        {
        if (static_cast<int>(e.first) < 10001)
            ASSERT_EQ(stmts[e.first].Prepare(db, e.second), BE_SQLITE_OK);
        else
            ASSERT_EQ(stmts[e.first].Prepare(dbv, e.second), BE_SQLITE_OK);
        }

    StopWatch s_insert(false);
    StopWatch s_update(false);
    StopWatch s_delete(false);
    StopWatch s_select(false);
    StopWatch v_insert(false);
    StopWatch v_update(false);
    StopWatch v_delete(false);
    StopWatch v_select(false);

    int max_rows = 200000;
    int p_prop_count = 5;
    int s_prop_count = 5;
    int v_prop_count = 10;
    //INSERT--------------------------------------------------------------------------------
    {
    auto& p = stmts[stype::foo_p_insert];
    auto& s = stmts[stype::foo_s_insert];
    auto& v = stmts[stype::foo_v_insert];
    LOG.infov("Running SQL Insert for %d rows", max_rows);
    s_insert.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        p.Reset();
        p.ClearBindings();
        p.BindInt64(1, id);
        p.BindInt(2, rand());
        for (int i = 0; i < p_prop_count; i++)
            p.BindInt(i + 3, rand());
        ASSERT_EQ(p.Step(), BE_SQLITE_DONE);

        s.Reset();
        s.ClearBindings();
        s.BindInt64(1, id);
        for (int i = 0; i < s_prop_count; i++)
            s.BindInt(i + 2, rand());

        ASSERT_EQ(s.Step(), BE_SQLITE_DONE);
        }
    s_insert.Stop();
    LOG.infov("Running Trigger Insert for %d rows", max_rows);
    v_insert.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        v.Reset();
        v.ClearBindings();
        v.BindInt64(1, id);
        v.BindInt(2, rand());
        for (int i = 0; i < v_prop_count; i++)
            v.BindInt(i + 3, rand());

        ASSERT_EQ(v.Step(), BE_SQLITE_DONE);
        }
    v_insert.Stop();
    }
    //UPDATE--------------------------------------------------------------------------------
    {
    auto& p = stmts[stype::foo_p_update];
    auto& s = stmts[stype::foo_s_update];
    auto& v = stmts[stype::foo_v_update];
    LOG.infov("Running SQL Update for %d rows", max_rows);
    s_update.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        p.Reset();
        p.ClearBindings();
        for (int i = 0; i < p_prop_count; i++)
            p.BindInt(i + 1, rand());

        p.BindInt64(p_prop_count + 1, id);
        ASSERT_EQ(p.Step(), BE_SQLITE_DONE);

        s.Reset();
        s.ClearBindings();
        for (int i = 0; i < s_prop_count; i++)
            s.BindInt(i + 1, rand());

        s.BindInt64(s_prop_count + 1, id);
        ASSERT_EQ(s.Step(), BE_SQLITE_DONE);
        }
    s_update.Stop();
    LOG.infov("Running Trigger Update for %d rows", max_rows);
    v_update.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        v.Reset();
        v.ClearBindings();
        for (int i = 0; i < v_prop_count; i++)
            v.BindInt(i + 1, rand());

        v.BindInt64(1, id);
        ASSERT_EQ(v.Step(), BE_SQLITE_DONE);
        }
    v_update.Stop();
    }
    //SELECT--------------------------------------------------------------------------------
    {
    auto& s = stmts[stype::foo_s_select];
    auto& v = stmts[stype::foo_v_select];
    LOG.infov("Running Sql Select for %d rows", max_rows);
    s_select.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        s.Reset();
        s.ClearBindings();
        s.BindInt64(1, id);
        ASSERT_EQ(s.Step(), BE_SQLITE_ROW);
        }
    s_select.Stop();
    LOG.infov("Running Trigger Select for %d rows", max_rows);
    v_select.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        v.Reset();
        v.ClearBindings();
        v.BindInt64(1, id);
        ASSERT_EQ(v.Step(), BE_SQLITE_ROW);
        }
    v_select.Stop();
    }
    //DELETE--------------------------------------------------------------------------------
    {
    auto& s = stmts[stype::foo_s_delete];
    auto& v = stmts[stype::foo_v_delete];
    LOG.infov("Running Sql Delete for %d rows", max_rows);
    s_delete.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        s.Reset();
        s.ClearBindings();
        s.BindInt64(1, id);
        ASSERT_EQ(s.Step(), BE_SQLITE_DONE);
        }
    s_delete.Stop();
    LOG.infov("Running Trigger Select for %d rows", max_rows);
    v_delete.Start();
    for (int id = 1; id <= max_rows; id++)
        {
        v.Reset();
        v.ClearBindings();
        v.BindInt64(1, id);
        ASSERT_EQ(v.Step(), BE_SQLITE_DONE);
        }
    v_delete.Stop();
    }

    auto diff = [] (StopWatch& a1, StopWatch& a2)
        {
        auto v1 = a1.GetElapsedSeconds();
        auto v2 = a2.GetElapsedSeconds();
        return ((v1 - v2) / ((v1 + v2) / 2.0)) * 100.0;
        };

    LOG.info("================================================");
    LOG.info("Operation    Trigger         Sql            Diff%");
    LOG.info("================================================");
    LOG.infov("SELECT       %.4f          %.4f           %.4f", v_select.GetElapsedSeconds(), s_select.GetElapsedSeconds(), diff(v_select, s_select));
    LOG.infov("INSERT       %.4f          %.4f           %.4f", v_insert.GetElapsedSeconds(), s_insert.GetElapsedSeconds(), diff(v_insert, s_insert));
    LOG.infov("UPDATE       %.4f          %.4f           %.4f", v_update.GetElapsedSeconds(), s_update.GetElapsedSeconds(), diff(v_update, s_update));
    LOG.infov("DELETE       %.4f          %.4f           %.4f", v_delete.GetElapsedSeconds(), s_delete.GetElapsedSeconds(), diff(v_delete, s_delete));
    }

END_ECDBUNITTESTS_NAMESPACE