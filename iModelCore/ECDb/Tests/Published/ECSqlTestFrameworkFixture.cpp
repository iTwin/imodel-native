/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestFrameworkFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFrameworkFixture.h"
#include "ECSqlAsserter.h"

using namespace std;

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       01/14
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlTestArg : NonCopyableClass
    {
    private:
        ECSqlTestItem const& m_testItem;
        ECSqlAsserterList const& m_asserterList;
        ECDb& m_aECDb;
        BeConditionVariable m_conditionVariable;

 
    public:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        ECSqlTestItem const& GetDatasetItem () const
            {
            return m_testItem;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        ECSqlAsserterList const& GetAsserters () const
            {
            return m_asserterList;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+-----
        ECDbR GetECDb ()
            {
            return m_aECDb;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        void Wait ()
            {
            m_conditionVariable.WaitOnCondition (nullptr, BeConditionVariable::Infinite);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        ECSqlTestArg (ECSqlTestItem const& aDatasetItem, ECSqlAsserterList const& aAsserterList, ECDbR aECDb)
            :m_testItem (aDatasetItem), m_asserterList (aAsserterList), m_aECDb (aECDb)
            {
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+-----
        BeConditionVariable& GetConditionVariable () { return m_conditionVariable; }
 
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       01/14
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlTestExecutor
    {
    private:

        std::vector<std::unique_ptr<ECSqlTestArg>> m_aECSqlArgs;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+-----
        THREAD_MAIN_DECL RunTest (void* args)
            {
            ECSqlTestArg* ecsqlArgs = static_cast<ECSqlTestArg*>(args);
            BeMutexHolder aGuard (ecsqlArgs->GetConditionVariable().GetMutex());

            const auto rollbackAfterwards = ecsqlArgs->GetDatasetItem ().GetRollbackAfterwards ();
            Savepoint perTestItemSavepoint (ecsqlArgs->GetECDb (), "AssertTestItem", rollbackAfterwards);

            for (auto const& asserter : ecsqlArgs->GetAsserters ())
                {
                asserter->Assert (ecsqlArgs->GetDatasetItem ());
                }

            if (rollbackAfterwards)
                perTestItemSavepoint.Cancel ();
            
            ecsqlArgs->GetConditionVariable ().notify_all();
            return 0;
            }
    public:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        unsigned Execute (ECSqlTestItem const& aDatasetItem, ECSqlAsserterList const& aAsserterList, ECDbR aECDb, bool async)
            {
            std::unique_ptr<ECSqlTestArg> args = std::unique_ptr<ECSqlTestArg> (new ECSqlTestArg (aDatasetItem, aAsserterList, aECDb));

            if (!async)
                RunTest (args.get ());
            else
                {
                BeThreadUtilities::StartNewThread (10000, ECSqlTestExecutor::RunTest, args.get ());
                if (!async)
                    args->Wait ();
                else
                    m_aECSqlArgs.push_back (move (args));
                }
            return 0;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        void WaitForAll ()
            {
            for (auto& aECSqlArg : m_aECSqlArgs)
                {
                aECSqlArg->Wait ();
                }

            m_aECSqlArgs.clear ();
            }

        ~ECSqlTestExecutor ()
            {
            WaitForAll ();
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTestFrameworkFixture::RunTest (ECSqlTestDataset const& dataset, ECSqlAsserterList const& asserters) const
    {
    Savepoint savepoint (GetECDb (), "RunTest", true);
    ECSqlTestExecutor executor;

    for (auto const& testItem : dataset.GetTestItems ())
        {
        executor.Execute (testItem, asserters, GetECDb (), false);
        }
    executor.WaitForAll ();
    //rollback any changes the tests might have made
    savepoint.Cancel ();
    }

//******************** ECSqlSelectTestFrameworkFixture *************************************

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlSelectTestFramework::SetUp()
    {
    SetupECDb("ecsqlselecttests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite), PerClassRowCount);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlSelectTestFramework::RunTest(ECSqlTestDataset const& dataset) const
    {
    ECSqlAsserterList asserters;
    asserters.push_back(unique_ptr<ECSqlAsserter>(new ECSqlSelectAsserter(GetECDb())));

    RunTest(dataset, asserters);
    }


//************** ECSqlNonSelectTestFrameworkFixture ********************************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlNonSelectTestFrameworkFixture::SetUp()
    {
    SetupECDb("ecsqlnonselecttests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(ECDb::OpenMode::ReadWrite), PerClassRowCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlNonSelectTestFrameworkFixture::RunTest(ECSqlTestDataset const& dataset) const
    {
    ECSqlAsserterList asserters;
    asserters.push_back(unique_ptr<ECSqlAsserter>(new ECSqlNonSelectAsserter(GetECDb())));

    RunTest(dataset, asserters);
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE