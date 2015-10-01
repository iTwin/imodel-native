/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCrudTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlCrudTestFixture.h"
#include "ECSqlStatementCrudAsserter.h"

using namespace std;

BEGIN_ECDBUNITTESTS_NAMESPACE

//******************** ECSqlCrudTestFixture *************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       01/14
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlTestArg : NonCopyableClass
    {
    private:
        ECSqlTestItem const& m_aDatasetItem;
        ECSqlCrudAsserterList const& m_aAsserterList;
        ECDb& m_aECDb;
        BeConditionVariable m_aConditionVariable;

 
    public:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        ECSqlTestItem const& GetDatasetItem () const
            {
            return m_aDatasetItem;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        ECSqlCrudAsserterList const& GetAsserters () const
            {
            return m_aAsserterList;
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
            m_aConditionVariable.WaitOnCondition (nullptr, BeConditionVariable::Infinite);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+------
        ECSqlTestArg (ECSqlTestItem const& aDatasetItem, ECSqlCrudAsserterList const& aAsserterList, ECDbR aECDb)
            :m_aDatasetItem (aDatasetItem), m_aAsserterList (aAsserterList), m_aECDb (aECDb)
            {
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Affan.Khan                       01/14
        //+---------------+---------------+---------------+---------------+---------------+-----
        BeConditionVariable& GetConditionVariable () { return m_aConditionVariable; }
 
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
        unsigned Execute (ECSqlTestItem const& aDatasetItem, ECSqlCrudAsserterList const& aAsserterList, ECDbR aECDb, bool async)
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
void ECSqlCrudTestFixture::RunTest (ECSqlTestDataset const& dataset, ECSqlCrudAsserterList const& asserters) const
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

//******************** ECSqlSelectTestFixture *************************************

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlSelectTestFixture::SetUp ()
    {
    SetUp ("ecsqlselecttests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams (Db::OpenMode::ReadWrite), PerClassRowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
ECDbR ECSqlSelectTestFixture::_SetUp (Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    SetupECDb (ecdbFileName, schemaECXmlFileName, openParams, perClassRowCount);
    return GetECDb();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlSelectTestFixture::RunTest (ECSqlTestDataset const& dataset) const
    {
    ECSqlCrudAsserterList asserters;
    asserters.push_back (unique_ptr<ECSqlCrudAsserter> (new ECSqlSelectStatementCrudAsserter (GetECDb ())));

    RunTest (dataset, asserters);
    }


//************** ECSqlNonSelectTestFixture ********************************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlNonSelectTestFixture::SetUp ()
    {
    SetUp ("ecsqlnonselecttests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams (ECDb::OpenMode::ReadWrite), PerClassRowCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECSqlNonSelectTestFixture::RunTest (ECSqlTestDataset const& dataset) const
    {
    ECSqlCrudAsserterList asserters;
    asserters.push_back (unique_ptr<ECSqlCrudAsserter> (new ECSqlNonSelectStatementCrudAsserter (GetECDb())));

    RunTest (dataset, asserters);
    }


END_ECDBUNITTESTS_NAMESPACE
