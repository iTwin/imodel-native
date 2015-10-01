/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlStatementCrudTestDatasetHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementCrudTestDatasetHelper.h"


BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlStatementCrudTestDatasetHelper::AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, int expectedResultColumnCount, int expectedResultRowCount /*= -1*/)
    {
    ECSqlTestItem testItem (ecsql);

    testItem.AddExpectedResult (PrepareECSqlExpectedResult::Create (nullptr));
    testItem.AddExpectedResult (ResultCountECSqlExpectedResult::Create (expectedResultColumnCount, expectedResultRowCount));

    return dataset.AddTestItem (testItem);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlStatementCrudTestDatasetHelper::AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, IECSqlExpectedResult::Category IECSqlBinder, Utf8CP description, int expectedResultColumnCount, int expectedResultRowCount /*= -1*/)
    {
    ECSqlTestItem testItem (ecsql);

    testItem.AddExpectedResult (PrepareECSqlExpectedResult::Create (nullptr, IECSqlBinder, description));
    testItem.AddExpectedResult (ResultCountECSqlExpectedResult::Create (IECSqlBinder, description, expectedResultColumnCount, expectedResultRowCount));

    return dataset.AddTestItem (testItem);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlStatementCrudTestDatasetHelper::AddNonSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, int expectedAffectedRowCount, bool rollbackAfterwards /*= false*/)
    {
    ECSqlTestItem testItem (ecsql, rollbackAfterwards);

    //In order to have the test prepare the statement before we can test the step, we need to add an expected success result for the preparation
    testItem.AddExpectedResult (PrepareECSqlExpectedResult::Create (nullptr));
    testItem.AddExpectedResult (AffectedRowsECSqlExpectedResult::Create (expectedAffectedRowCount));

    return dataset.AddTestItem (testItem);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (ECSqlTestDataset& dataset, Utf8CP ecsql, IECSqlExpectedResult::Category failureCategory, Utf8CP description /*= nullptr*/)
    {
    ECSqlTestItem testItem (ecsql);
    testItem.AddExpectedResult (PrepareECSqlExpectedResult::CreateFailing (nullptr, failureCategory, description));

    return dataset.AddTestItem (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlStatementCrudTestDatasetHelper::AddStepFailingNonSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, IECSqlExpectedResult::Category failureCategory, Utf8CP description /*= nullptr*/, bool rollbackAfterwards /* = false*/)
    {
    ECSqlTestItem testItem (ecsql, rollbackAfterwards);
    //In order to have the test prepare the statement before we can test the step, we need to add an expected success result for the preparation
    testItem.AddExpectedResult (PrepareECSqlExpectedResult::Create (nullptr));
    testItem.AddExpectedResult (AffectedRowsECSqlExpectedResult::CreateFailing (failureCategory, description));

    return dataset.AddTestItem (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECInstanceId ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    auto stat = stmt.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        EXPECT_EQ(ECSqlStatus::Success, stat) << "Inserting test instance with '" << ecsql << "' failed. Preparation failed";
        return ECInstanceId ();
        }

    ECInstanceKey newECInstanceKey;
    DbResult stepStat = stmt.Step (newECInstanceKey);
    if (stepStat != BE_SQLITE_DONE)
        {
        EXPECT_EQ (BE_SQLITE_DONE, stepStat) << "Inserting test instance with '" << ecsql << "' failed. Step failed";
        return ECInstanceId ();
        }
    else
        return newECInstanceKey.GetECInstanceId ();
    }

END_ECDBUNITTESTS_NAMESPACE
