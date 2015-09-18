/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlStatementCrudTestDatasetHelper.h $
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
struct ECSqlStatementCrudTestDatasetHelper
    {
private:
    //static class
    ECSqlStatementCrudTestDatasetHelper ();
    ~ECSqlStatementCrudTestDatasetHelper ();

public:
    static ECSqlTestItem& AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, int expectedResultColumnCount, int expectedResultRowCount = -1);
    static ECSqlTestItem& AddSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, IECSqlExpectedResult::Category IECSqlBinder, Utf8CP description, int expectedResultColumnCount, int expectedResultRowCount = -1);
    static ECSqlTestItem& AddNonSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, int expectedAffectedRowCount, bool rollbackAfterwards = false);
    
    static ECSqlTestItem& AddPrepareFailing (ECSqlTestDataset& dataset, Utf8CP ecsql, IECSqlExpectedResult::Category failureCategory, Utf8CP description = nullptr);
    static ECSqlTestItem& AddStepFailingNonSelect (ECSqlTestDataset& dataset, Utf8CP ecsql, IECSqlExpectedResult::Category failureCategory, Utf8CP description = nullptr, bool rollbackAfterwards = false);

    //Helpers
    static ECInstanceId InsertTestInstance (ECDbTestProject&, Utf8CP ecsql);
    static ECN::ECClassId GetClassId (ECDbTestSchemaManager const& schemaManager, Utf8CP schemaName, Utf8CP className);
    };

END_ECDBUNITTESTS_NAMESPACE