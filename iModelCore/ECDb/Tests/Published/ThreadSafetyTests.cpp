/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ThreadSafetyTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <rapidjson/BeRapidJson.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

THREAD_MAIN_IMPL __Run(void* args);

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
struct BeThread : NonCopyableClass
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
        struct __ThreadArg : NonCopyableClass, IConditionVariablePredicate
            {
            private:
                std::function<void(void)> m_worker;
                intptr_t m_threadId;
                BeAtomic<State> m_stat;
                BeConditionVariable m_conditionalVar;
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
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.ecschema.xml")));
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
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.ecschema.xml")));
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
                ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::Readonly)));

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
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.ecschema.xml")));
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
                ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::Readonly)));

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

END_ECDBUNITTESTS_NAMESPACE
