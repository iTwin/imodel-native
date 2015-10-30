/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestFrameworkHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestDataset.h"
#include "ECSqlExpectedResultImpls.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                           Krischan.Eberle            07/13
//=======================================================================================    
struct ECSqlTestFrameworkHelper
    {
private:
    //static class
    ECSqlTestFrameworkHelper ();
    ~ECSqlTestFrameworkHelper ();

public:
    static ECSqlTestItem& AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, int expectedResultColumnCount, int expectedResultRowCount = -1);
    static ECSqlTestItem& AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, ECSqlExpectedResult::Category IECSqlBinder, Utf8CP description, int expectedResultColumnCount, int expectedResultRowCount = -1);
    static ECSqlTestItem& AddNonSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, bool rollbackAfterwards = false);
    
    static ECSqlTestItem& AddPrepareFailing (ECSqlTestDataset& dataset, Utf8CP ecsql, ECSqlExpectedResult::Category failureCategory, Utf8CP description = nullptr);
    static ECSqlTestItem& AddStepFailingNonSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, ECSqlExpectedResult::Category failureCategory, Utf8CP description = nullptr, bool rollbackAfterwards = false);

    //Helpers
    static ECInstanceId InsertTestInstance (ECDbCR, Utf8CP ecsql);
    };

END_ECDBUNITTESTS_NAMESPACE