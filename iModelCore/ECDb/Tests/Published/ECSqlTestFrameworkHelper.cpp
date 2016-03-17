/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestFrameworkHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFrameworkHelper.h"

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlTestFrameworkHelper::AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, int expectedResultColumnCount, int expectedResultRowCount /*= -1*/)
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
ECSqlTestItem& ECSqlTestFrameworkHelper::AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, ECSqlExpectedResult::Category IECSqlBinder, Utf8CP description, int expectedResultColumnCount, int expectedResultRowCount /*= -1*/)
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
ECSqlTestItem& ECSqlTestFrameworkHelper::AddNonSelect(ECSqlTestDataset& dataset, Utf8CP ecsql, bool rollbackAfterwards /*= false*/)
    {
    ECSqlTestItem testItem(ecsql, rollbackAfterwards);

    //In order to have the test prepare the statement before we can test the step, we need to add an expected success result for the preparation
    testItem.AddExpectedResult(PrepareECSqlExpectedResult::Create(nullptr));
    testItem.AddExpectedResult(ECSqlExpectedResult::Create());

    return dataset.AddTestItem(testItem);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlTestFrameworkHelper::AddPrepareFailing (ECSqlTestDataset& dataset, Utf8CP ecsql, ECSqlExpectedResult::Category failureCategory, Utf8CP description /*= nullptr*/)
    {
    ECSqlTestItem testItem (ecsql);
    testItem.AddExpectedResult (PrepareECSqlExpectedResult::CreateFailing (nullptr, failureCategory, description));

    return dataset.AddTestItem (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlTestFrameworkHelper::AddStepFailingNonSelect(ECSqlTestDataset& dataset, Utf8CP ecsql, ECSqlExpectedResult::Category failureCategory, Utf8CP description /*= nullptr*/, bool rollbackAfterwards /* = false*/)
    {
    ECSqlTestItem testItem(ecsql, rollbackAfterwards);
    //In order to have the test prepare the statement before we can test the step, we need to add an expected success result for the preparation
    testItem.AddExpectedResult(PrepareECSqlExpectedResult::Create(nullptr));
    testItem.AddExpectedResult(ECSqlExpectedResult::CreateFailing(failureCategory, description));

    return dataset.AddTestItem(testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECInstanceId ECSqlTestFrameworkHelper::InsertTestInstance (ECDbCR ecdb, Utf8CP ecsql)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
bvector<ECInstanceId> ECSqlTestFrameworkHelper::GetValidECInstanceIds (ECDbCR ecdb, Utf8CP ecsql)
    {
    bvector<ECInstanceId> instanceIds;
    ECSqlStatement stmt;
    EXPECT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, ecsql));
    while (DbResult::BE_SQLITE_ROW == stmt.Step ())
        {
        instanceIds.push_back(stmt.GetValueId<ECInstanceId>(0));
        }

    return instanceIds;
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE