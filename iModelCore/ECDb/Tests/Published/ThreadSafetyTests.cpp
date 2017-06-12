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
        BeThread()
            {}

    public:
        ~BeThread() {}
        BeThread(BeThread && rhs)
            :m_arg(std::move(rhs.m_arg))
            {}
        BeThread& operator = (BeThread && rhs)
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
                {
                m_arg->Join();
                }
            }
        void Join(uint32_t timeoutMillis) const
            {
            if (m_arg)
                {
                m_arg->Join(timeoutMillis);
                }

            }
        intptr_t GetId() const
            {
            if (m_arg)
                {
                return m_arg->GetId();
                }
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
            if (BeThreadUtilities::StartNewThread(DEFAULT_STACK_SIZE, __Run, thread.m_arg.get()) != SUCCESS)
                {
                thread.m_arg->GetStateR().store(BeThread::State::Error);
                }

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
struct ThreadSafetyTests : public ECDbTestFixture
    {};


//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                       12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ThreadSafetyTests, AllThreadsShareDb)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), Db::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));

    Utf8CP ecsql = "SELECT Name, NumberOfEmployees FROM stco.Company";

    std::vector<BeThread> threads;

    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start(
            [&] ()
            {
            printf("Begin here: %Id ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
                int rowCount = 0;
                while (stmt.Step() != BE_SQLITE_DONE)
                    {
                    EXPECT_STREQ("Sample string", stmt.GetValueText(0));
                    EXPECT_EQ(123, stmt.GetValueInt(1));
                    rowCount++;
                    }
                EXPECT_EQ(3, rowCount); // 3 rows entered for Company above
                stmt.Finalize();
                }
            printf("End here: %Id ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
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
TEST_F(ThreadSafetyTests, MultiThreadsOpenDb_ECSQL)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));
    BeFileName ecdbFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP ecsql = "SELECT Name, NumberOfEmployees FROM stco.Company";

    std::vector<BeThread> threads;

    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start(
            [&] ()
            {
            printf("Begin here: %Id ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECDb db;
                DbResult stat = db.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::Readonly));
                EXPECT_EQ(BE_SQLITE_OK, stat);

                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, ecsql));
                int rowCount = 0;
                while (stmt.Step() != BE_SQLITE_DONE)
                    {
                    EXPECT_STREQ("Sample string", stmt.GetValueText(0));
                    EXPECT_EQ(123, stmt.GetValueInt(1));
                    rowCount++;
                    }
                EXPECT_EQ(3, rowCount); // 3 rows entered for Company above
                stmt.Finalize();
                }
            printf("End here: %Id ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
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
TEST_F(ThreadSafetyTests, MultiThreadsOpenDb_SQL)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));
    BeFileName ecdbFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    Utf8CP sql = "SELECT Name, NumberOfEmployees FROM sc_Company";

    std::vector<BeThread> threads;

    const uint32_t kThreadCount = BeThreadUtilities::GetHardwareConcurrency();
    int counter = 0;
    for (uint32_t i = 0; i < kThreadCount; ++i)
        {
        threads.push_back(BeThread::Start(
            [&] ()
            {
            printf("Begin here: %Id ===========================\n ", BeThreadUtilities::GetCurrentThreadId());

            for (int i = 0; i < 100; ++i)
                {
                ECDb db;
                DbResult stat = db.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(Db::OpenMode::Readonly));
                EXPECT_EQ(BE_SQLITE_OK, stat);

                Statement stmt;
                EXPECT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(db, sql));
                int rowCount = 0;
                while (stmt.Step() != BE_SQLITE_DONE)
                    {
                    EXPECT_STREQ("Sample string", stmt.GetValueText(0));
                    EXPECT_EQ(123, stmt.GetValueInt(1));
                    rowCount++;
                    }
                EXPECT_EQ(3, rowCount); // 3 rows entered for Company above
                stmt.Finalize();
                }
            printf("End here: %Id ===========================\n ", BeThreadUtilities::GetCurrentThreadId());
            }
        ));
        counter++;
        }

    BeThread::JoinAll(threads);
    EXPECT_EQ(counter, kThreadCount);
    }

END_ECDBUNITTESTS_NAMESPACE
