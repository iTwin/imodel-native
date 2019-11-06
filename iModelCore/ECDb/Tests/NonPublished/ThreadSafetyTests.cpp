/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeRapidJson/BeRapidJson.h>
#include <ECObjects/ECObjects.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

THREAD_MAIN_IMPL __Run(void* args);

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
struct BeThread final
    {
    friend THREAD_MAIN_IMPL __Run(void* args);
    enum class State
        {
        NotStarted,
        Started,
        Stop,
        Error,
        };

    private:
        struct __ThreadArg : IConditionVariablePredicate
            {
            private:
                std::function<void(void)> m_worker;
                intptr_t m_threadId;
                BeAtomic<State> m_stat;
                BeConditionVariable m_conditionalVar;

                //not copyable
                __ThreadArg(__ThreadArg const&) = delete;
                __ThreadArg& operator=(__ThreadArg const&) = delete;

            private:
                bool _TestCondition(struct BeConditionVariable &cv) override
                    {
                    return m_stat.load() == BeThread::State::Stop || m_stat.load() == BeThread::State::Error;
                    }
            public:
                __ThreadArg(std::function<void(void)> worker)
                    :m_worker(worker), m_threadId(0)
                    {
                    m_stat.store(BeThread::State::NotStarted);
                    }
                ~__ThreadArg() {}
                BeAtomic<State> & GetStateR() { return m_stat; }
                void Invoke() { m_threadId = BeThreadUtilities::GetCurrentThreadId();  m_worker(); }
                BeConditionVariable& GetConditionVariableR() { return m_conditionalVar; }
                void Join(uint32_t timeoutMillis) { GetConditionVariableR().WaitOnCondition(this, timeoutMillis); }
                void Join() { Join(BeConditionVariable::Infinite); }
                intptr_t GetId() const { return m_threadId; }

            };

    private:
        const static int DEFAULT_STACK_SIZE = 1024 * 1024 * 8 * 2;
        mutable std::unique_ptr<__ThreadArg> m_arg;
        
        BeThread() {}
        //not copyable
        BeThread(BeThread const&) = delete;
        BeThread& operator=(BeThread const&) = delete;

    public:
        BeThread(BeThread&& rhs) :m_arg(std::move(rhs.m_arg)) {}
        ~BeThread() {}

        BeThread& operator=(BeThread && rhs)
            {
            if (this != &rhs)
                {
                m_arg = std::move(rhs.m_arg);
                }
            return *this;
            }

        State GetState() const { return m_arg->GetStateR().load(); }
        void Join() const
            {
            if (m_arg)
                m_arg->Join();
            }
        void Join(uint32_t timeoutMillis) const
            {
            if (m_arg)
                m_arg->Join(timeoutMillis);
            }
        intptr_t GetId() const
            {
            if (m_arg)
                return m_arg->GetId();

            return 0;
            }
        static void JoinAll(std::vector<BeThread> const& threads, uint32_t timeoutMillis)
            {
            for (BeThread const& thread : threads)
                {
                thread.Join(timeoutMillis);
                }
            }
        static void JoinAll(std::vector<BeThread> const& threads)
            {
            JoinAll(threads, BeConditionVariable::Infinite);
            }

        static BeThread Start(std::function<void(void)> work)
            {
            BeThread thread;
            thread.m_arg = std::unique_ptr<__ThreadArg>(new __ThreadArg(work));
            if (BeThreadUtilities::StartNewThread(__Run, thread.m_arg.get(), DEFAULT_STACK_SIZE) != SUCCESS)
                thread.m_arg->GetStateR().store(BeThread::State::Error);

            return thread;
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
THREAD_MAIN_IMPL __Run(void* args)
    {
    BeThread::__ThreadArg* threadArg = static_cast<BeThread::__ThreadArg*>(args);
    BeAssert(threadArg != nullptr);
    BeMutexHolder holder(threadArg->GetConditionVariableR().GetMutex());
    threadArg->GetStateR().store(BeThread::State::Started);
    threadArg->Invoke();
    threadArg->GetStateR().store(BeThread::State::Stop);
    threadArg->GetConditionVariableR().notify_all();
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Majd.Uddin                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ThreadSafetyTests : public ECDbTestFixture  {};


//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTests, MultipleThreadsSingleConnection)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    ECInstanceKey acmeKey, bentleyKey, vwKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(acmeKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('ACME', 123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(bentleyKey,"INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('Bentley', 4000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(vwKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('VW', 100000)"));

    std::vector<BeThread> threads;
    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start([&] ()
            {
            printf("Thread %Id starts ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, NumberOfEmployees FROM stco.Company"));
                int rowCount = 0;
                while (stmt.Step() == BE_SQLITE_ROW)
                    {
                    ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
                    Utf8CP name = stmt.GetValueText(1);
                    const int employeeCount = stmt.GetValueInt(2);

                    if (id == acmeKey.GetInstanceId())
                        {
                        ASSERT_STREQ("ACME", name);
                        ASSERT_EQ(123, employeeCount);
                        }
                    else if (id == bentleyKey.GetInstanceId())
                        {
                        ASSERT_STREQ("Bentley", name);
                        ASSERT_EQ(4000, employeeCount);
                        }
                    else if (id == vwKey.GetInstanceId())
                        {
                        ASSERT_STREQ("VW", name);
                        ASSERT_EQ(100000, employeeCount);
                        }
                    else
                        FAIL();

                    rowCount++;
                    }
                ASSERT_EQ(3, rowCount); // 3 rows entered for Company above
                stmt.Finalize();
                }
            printf("Thread %Id ends ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
            }
        ));
        counter++;
        }

    BeThread::JoinAll(threads);
    EXPECT_EQ(counter, kThreadCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTests, MultipleThreadsSingleConnection_FailingGetClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ForwardCompatibilitySafeguards.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="A">
                    <ECProperty propertyName="Prop1" typeName="string" />
                    <ECProperty propertyName="Prop2" typeName="int" />
                </ECEntityClass>
         </ECSchema>)xml")));

    //modify the primitive type of Prop2 to an invalid value
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Property SET PrimitiveType=-1 WHERE Name='Prop2'"));
    m_ecdb.SaveChanges();
    ReopenECDb();

    std::vector<BeThread> threads;
    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start([&] ()
            {
            printf("Thread %Id starts ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
                EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown property primitive type";

                EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
                }

            printf("Thread %Id ends ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
            }
        ));
        counter++;
        }

    BeThread::JoinAll(threads);
    EXPECT_EQ(counter, kThreadCount);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTests, ConnectionPerThread_ECSQL)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    ECInstanceKey acmeKey, bentleyKey, vwKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(acmeKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('ACME', 123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(bentleyKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('Bentley', 4000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(vwKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('VW', 100000)"));

    BeFileName ecdbFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    std::vector<BeThread> threads;
    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start([&] ()
            {
            printf("Thread %Id starts ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECDb db;
                ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(ecdbFileName, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT ECInstanceId, Name, NumberOfEmployees FROM stco.Company"));
                int rowCount = 0;
                while (stmt.Step() == BE_SQLITE_ROW)
                    {
                    ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
                    Utf8CP name = stmt.GetValueText(1);
                    const int employeeCount = stmt.GetValueInt(2);

                    if (id == acmeKey.GetInstanceId())
                        {
                        ASSERT_STREQ("ACME", name);
                        ASSERT_EQ(123, employeeCount);
                        }
                    else if (id == bentleyKey.GetInstanceId())
                        {
                        ASSERT_STREQ("Bentley", name);
                        ASSERT_EQ(4000, employeeCount);
                        }
                    else if (id == vwKey.GetInstanceId())
                        {
                        ASSERT_STREQ("VW", name);
                        ASSERT_EQ(100000, employeeCount);
                        }
                    else
                        FAIL();

                    rowCount++;
                    }
                ASSERT_EQ(3, rowCount); // 3 rows entered for Company above
                stmt.Finalize();
                }
            printf("Thread %Id ends ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
            }
        ));
        counter++;
        }

    BeThread::JoinAll(threads);
    EXPECT_EQ(counter, kThreadCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTests, ConnectionPerThread_SQL)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    BeFileName ecdbFileName(m_ecdb.GetDbFileName());

    ECInstanceKey acmeKey, bentleyKey, vwKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(acmeKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('ACME', 123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(bentleyKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('Bentley', 4000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(vwKey, "INSERT INTO stco.Company(Name,NumberOfEmployees) VALUES('VW', 100000)"));
    m_ecdb.CloseDb();

    std::vector<BeThread> threads;
    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start([&] ()
            {
            printf("Thread %Id starts ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECDb db;
                ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(ecdbFileName, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

                Statement stmt;
                ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT Id, Name, NumberOfEmployees FROM sc_Company"));
                int rowCount = 0;
                while (stmt.Step() == BE_SQLITE_ROW)
                    {
                    ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
                    Utf8CP name = stmt.GetValueText(1);
                    const int employeeCount = stmt.GetValueInt(2);

                    if (id == acmeKey.GetInstanceId())
                        {
                        ASSERT_STREQ("ACME", name);
                        ASSERT_EQ(123, employeeCount);
                        }
                    else if (id == bentleyKey.GetInstanceId())
                        {
                        ASSERT_STREQ("Bentley", name);
                        ASSERT_EQ(4000, employeeCount);
                        }
                    else if (id == vwKey.GetInstanceId())
                        {
                        ASSERT_STREQ("VW", name);
                        ASSERT_EQ(100000, employeeCount);
                        }
                    else
                        FAIL();

                    rowCount++;
                    }
                ASSERT_EQ(3, rowCount); // 3 rows entered for Company above
                stmt.Finalize();
                }
            printf("Thread %Id ends ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
            } 
        ));
        counter++;
        }

    BeThread::JoinAll(threads);
    EXPECT_EQ(counter, kThreadCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       12/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTests, ConncurentECSqlStressTest)
    {
    const int noOfClasses = 6000;
    const int noOfPropertiesPerClass = 10;
    const int rowPerClass = 5;
    const int nthreads = BeThreadUtilities::GetHardwareConcurrency();
    const int threadTask = noOfClasses / nthreads;

    ECDb ecdb;
    BeFileName ecdbFileName = BuildECDbPath("LargeSchema.ecdb");
    if (!ecdbFileName.DoesPathExist())
        {
        printf("Creating file ... %s\r\n", ecdbFileName.GetNameUtf8().c_str());
        ASSERT_EQ(BE_SQLITE_OK,  ecdb.CreateNewDb(ecdbFileName));
        ECSchemaPtr testSchema;        
        ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 0));
        ECSchemaP ecdbMapRef = const_cast<ECSchemaP>(ecdb.Schemas().GetSchema("ECDbMap"));        
        ASSERT_EQ(ECObjectsStatus::Success, testSchema->AddReferencedSchema(*ecdbMapRef));
        StandaloneECInstancePtr classMapCA = ecdbMapRef->GetClassCP("ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_EQ(ECObjectsStatus::Success, classMapCA->SetValue("MapStrategy", ECValue("TablePerHierarchy")));
        StandaloneECInstancePtr sharedColumnCA = ecdbMapRef->GetClassCP("ShareColumns")->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_EQ(ECObjectsStatus::Success, sharedColumnCA->SetValue("MaxSharedColumnsBeforeOverflow", ECValue(noOfPropertiesPerClass)));
        ASSERT_EQ(ECObjectsStatus::Success, sharedColumnCA->SetValue("ApplyToSubclassesOnly", ECValue(false)));
        ECEntityClassP baseClass;
        ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*classMapCA));
        ASSERT_EQ(ECObjectsStatus::Success, baseClass->SetCustomAttribute(*sharedColumnCA));
        for (int nClass = 1; nClass <= noOfClasses; nClass++)
            {
            ECEntityClassP entityClass;
            Utf8String entityClasName;
            entityClasName.Sprintf("c_%d", nClass);
            ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(entityClass, entityClasName));
            ASSERT_EQ(ECObjectsStatus::Success, entityClass->AddBaseClass(*baseClass));
            for (int nProperty = 1; nProperty <= noOfPropertiesPerClass; nProperty++)
                {
                PrimitiveECPropertyP primitiveProperty;
                Utf8String propertyName;
                propertyName.Sprintf("p_%d_%d", nClass, nProperty);
                ASSERT_EQ(ECObjectsStatus::Success, entityClass->CreatePrimitiveProperty(primitiveProperty, propertyName, PrimitiveType::PRIMITIVETYPE_Long));
                }
            }

        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({testSchema.get()}));
        for (ECClassCP derivedClass : baseClass->GetDerivedClasses())
            {
            ECSqlStatement stmt;
            Utf8String sql, values;
            sql = "INSERT INTO ts." + derivedClass->GetName() + " (";
            values = "VALUES (";
            bool first = true;
            for (ECPropertyCP localProp : derivedClass->GetProperties())
                {
                if (first)
                    first = false;
                else
                    {
                    sql.append(",");
                    values.append(",");
                    }

                sql.append(localProp->GetName());
                values.append("?");
                }
            sql.append(")").append(values).append(")");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, sql.c_str()));
            for (int r = 0; r < rowPerClass; r++)
                {
                stmt.Reset();
                stmt.ClearBindings();
                for (int i = 1; i <= noOfPropertiesPerClass; i++)
                    stmt.BindInt(i, rand());

                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                }
            }
        ecdb.CloseDb();
        }

    struct TestSQLFunc : ScalarFunction
        {
        protected:
            ECDbCR m_conn;
            const std::vector<Utf8String> m_classes;
            virtual void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
                {
                int k = 0;
                for (int i = 0; i < 3; i++)
                    {
                    //printf("Start TestSQLFunc ... [thread=%d, task=%d]\r\n", (int)BeThreadUtilities::GetCurrentThreadId(), i);
                    ECSqlStatement stmt;
                    const size_t n = (size_t) floor(((double) rand() / RAND_MAX)*m_classes.size());
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_conn, SqlPrintfString("SELECT * FROM %s", m_classes[n].c_str())));
                    while (stmt.Step() == BE_SQLITE_ROW)
                        {
                        k = k + 1;
                        }
                    //printf("End TestSQLFunc   ... [thread=%d, task=%d]\r\n", (int) BeThreadUtilities::GetCurrentThreadId(), i);
                    }

                ctx.SetResultInt(k);
                }
        public:
            TestSQLFunc(ECDbCR conn, std::vector<Utf8String> classes) : ScalarFunction("testSqlFunc", 0, DbValueType::IntegerVal), m_conn(conn), m_classes(classes) {}
        };

    std::vector<Utf8String> classes;
    //////////////////////////////////////////////   
    {
    printf("Reading class names\r\n");
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::ReadWrite)));
    Statement stmt;  
    stmt.Prepare(ecdb, R"(
        SELECT [S].[Alias] || '.' || [C].[Name]
        FROM   [ec_Class] [C]
               INNER JOIN [ec_Schema] [S] ON [S].[Id] = [C].[SchemaId]
        WHERE  [S].[Alias] = 'ts';)");

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        classes.push_back(stmt.GetValueText(0));
        }

    stmt.Finalize();
    ecdb.CloseDb();
    }
    printf("Running threads\r\n");
    //////////////////////////////////////////////
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::ReadWrite)));
    TestSQLFunc testFunc(ecdb, classes);
    ecdb.AddFunction(testFunc);
    std::vector<BeThread> threads;
    for (int p = 0; p < nthreads; p++)
        {
        threads.push_back(BeThread::Start([&] ()
            {
            for (int i = 0; i < threadTask; i++)
                {
                //printf("Start Task        ... [thread=%d, task=%d]\r\n", (int) BeThreadUtilities::GetCurrentThreadId(),i);
                ECSqlStatement stmt;
                const size_t n = (size_t) floor(((double) rand() / RAND_MAX)*classes.size());
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, SqlPrintfString("SELECT *,testSqlFunc() FROM %s", classes[n].c_str())));
                while (stmt.Step() == BE_SQLITE_ROW)
                    {
                    }
                //printf("End Task          ... [thread=%d, task=%d]\r\n", (int) BeThreadUtilities::GetCurrentThreadId(), i);
                }
            }));
        }

    BeThread::JoinAll(threads);
    ecdb.RemoveFunction(testFunc);
    }

struct ThreadSafetyTestsToRun : public ECDbTestFixture {};
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       12/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTestsToRun, UnitsLoadAndEvaluateCorrectlyInManyThreads)
    {
    const int nthreads = BeThreadUtilities::GetHardwareConcurrency();

    ECDb ecdb;
    BeFileName ecdbFileName = BuildECDbPath("WithUnits.ecdb");
    if (!ecdbFileName.DoesPathExist())
        {
        printf("Creating file ... %s\r\n", ecdbFileName.GetNameUtf8().c_str());
        ASSERT_EQ(BE_SQLITE_OK,  ecdb.CreateNewDb(ecdbFileName));
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("Units", 1, 0, 0);
        ECSchemaPtr unitsSchema = context->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
        ASSERT_TRUE(unitsSchema.IsValid());
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({ unitsSchema.get() }));
        ecdb.CloseDb();
        }

    printf("Running threads\r\n");
    //////////////////////////////////////////////
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::Readonly)));
    bvector<ECSchemaCP> schemas = ecdb.Schemas().GetSchemas(true);
    auto unitsIt = std::find_if(schemas.begin(), schemas.end(), [](ECSchemaCP schema) { return schema->GetName().EqualsI("Units"); });
    ASSERT_TRUE(unitsIt != schemas.end());

    ECUnitCP psig = (*unitsIt)->GetUnitCP("PSIG");
    ASSERT_NE(nullptr, psig);
    ECUnitCP pa = (*unitsIt)->GetUnitCP("PA");
    ASSERT_NE(nullptr, pa);

    ECUnitCP gph = (*unitsIt)->GetUnitCP("GALLON_PER_HR");
    ASSERT_NE(nullptr, gph);
    ECUnitCP cmps = (*unitsIt)->GetUnitCP("CUB_M_PER_SEC");
    ASSERT_NE(nullptr, cmps);

    ECUnitCP f = (*unitsIt)->GetUnitCP("FAHRENHEIT");
    ASSERT_NE(nullptr, f);
    ECUnitCP c = (*unitsIt)->GetUnitCP("CELSIUS");
    ASSERT_NE(nullptr, c);

    std::vector<BeThread> threads;
    for (int p = 0; p < nthreads; p++)
        {
        threads.push_back(BeThread::Start([&] ()
            {
            double convertedValue;
            EXPECT_TRUE(Units::UnitsProblemCode::NoProblem == psig->Convert(convertedValue, 42.42, pa));
            EXPECT_TRUE(convertedValue != 0) << "psig to pa conversion failed";

            EXPECT_TRUE(Units::UnitsProblemCode::NoProblem == gph->Convert(convertedValue, 42.42, cmps));
            EXPECT_TRUE(convertedValue != 0) << "gph to cmps conversion failed";

            EXPECT_TRUE(Units::UnitsProblemCode::NoProblem == f->Convert(convertedValue, 42.42, c));
            EXPECT_TRUE(convertedValue != 0) << "f to c conversion failed";
            }));
        }

    BeThread::JoinAll(threads);
    }
END_ECDBUNITTESTS_NAMESPACE
