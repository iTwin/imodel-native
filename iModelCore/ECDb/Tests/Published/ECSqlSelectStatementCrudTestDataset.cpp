/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlSelectStatementCrudTestDataset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlSelectStatementCrudTestDataset.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::AliasTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;
    Utf8CP ecsql = "SELECT ECInstanceId, PStructProp A11 FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2);

    //tests when class alias is same as a property name. This should work unless the property is a struct property
    ecsql = "SELECT S.ECInstanceId FROM ecsql.PSA S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM ecsql.PSA S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM ecsql.PSA S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.I FROM ecsql.PSA S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM (SELECT S FROM ecsql.PSA) S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM (SELECT S FROM ecsql.PSA) S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM (SELECT S, I FROM ecsql.PSA) S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.I FROM (SELECT S, I FROM ecsql.PSA) S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM ecsql.PSA, (SELECT ECInstanceId, I, S FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.Bla FROM ecsql.PSA, (SELECT ECInstanceId, I, 3.14 AS Bla FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM ecsql.PSA, (SELECT I, S FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStructProp FROM ecsql.PSA PStructProp";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.ECInstanceId FROM ecsql.PSA PStructProp";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.s FROM ecsql.PSA PStructProp";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.I FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.PStructProp FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.i FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::ArrayTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT Dt_Array, B FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array = ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported, "Arrays are not supported yet in where clause.");

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE CARDINALITY (Dt_Array) > 0";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT S, PStruct_Array FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStruct_Array = ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported, "Struct arrays are not supported yet in where clause.");

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStruct_Array IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported, "Struct arrays are not supported yet in where clause.");

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStruct_Array IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported, "Struct arrays are not supported yet in where clause.");
    
    ecsql = "SELECT I, S FROM ecsql.PSA WHERE CARDINALITY (PStruct_Array) > 0";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT Dt_Array[1], B FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT Dt_Array[100000], B FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT Dt_Array[-1], B FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT UnknowProperty[1], B FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT UnknowProperty[-1], B FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::BetweenOperatorTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I BETWEEN 1 AND 3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I BETWEEN 122 AND 124";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN 1 AND 3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    //S always amounts to Sample String
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S BETWEEN 'Q' AND 'T'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    //S always amounts to Sample String
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S BETWEEN 'Q' AND 'R'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN ? AND 3";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN 1 AND ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN ? AND ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (100)));
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::CastTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;
    Utf8CP ecsql = "SELECT CAST (S AS BINARY) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS BINARY) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS BINARY) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (PStructProp AS BINARY) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS BINARY) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
   

    ecsql = "SELECT CAST (B AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (1 AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (I AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (True AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (False AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST ('1' AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (Unknown AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "SQL-99 keyword UNKNOWN not supported in ECSQL as ECObjects doesn't have a counterpart for it.");

    ecsql = "SELECT CAST (NULL AS BOOLEAN) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (B AS BOOL) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Only keyword BOOLEAN supported.");

    ecsql = "SELECT CAST (Bi AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (Dt AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS DATETIME) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DtUtc AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DtUtc AS DATETIME) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DtUtc AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (TIMESTAMP '2013-02-09T12:00:00' AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (TIMESTAMP '2013-02-09T12:00:00' AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DATE '2013-02-09' AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (D AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (123425 AS DATETIME) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (123425.2343 AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (True AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (123425.123 AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (S AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P2D AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS DATE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    
    ecsql = "SELECT CAST (Bi AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (L AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P2D AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS DOUBLE) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    
    ecsql = "SELECT CAST (Bi AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (False AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS INT32) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    
    ecsql = "SELECT CAST (Bi AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (L AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (L AS INT64) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (True AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS LONG) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


    ecsql = "SELECT CAST (B AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (True AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (False AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS TEXT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "TEXT is SQLite specific datatype which is not supported by SQLite.");

    ecsql = "SELECT CAST (NULL AS STRING) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


    ecsql = "SELECT CAST (Bi AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (I AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (D AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (S AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (I AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (D AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (S AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


    ecsql = "SELECT CAST (D AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (3.134 AS POINT2D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (100000123 AS POINT3D) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);


    ecsql = "SELECT CAST (L AS ecsql.PStruct) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS ecsql.PStruct) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT CAST (NULL AS ecsql.PStruct) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);


    //Support for geometry is not fleshed out yet. Possible type names are tested here
    ecsql = "SELECT CAST (I AS IGeometry) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (I AS Geometry) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS IGeometry) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS Geometry) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);


    ecsql = "SELECT CAST (S AS I) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (? AS INT) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT CAST (I AS ?) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);


    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::CommonGeometryTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Geometry, S FROM ecsql.PASpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Geometry_Array, S FROM ecsql.PASpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PASpatial ORDER BY Geometry";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Common Geometry properties cannot be ordered by ECSQL");

    ecsql = "SELECT * FROM ecsql.PASpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 7, rowCountPerClass);

    ecsql = "SELECT p.* FROM ecsql.PASpatial p";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 7, rowCountPerClass);

    ecsql = "SELECT count(*) FROM ecsql.PASpatial p";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT * FROM ecsql.SSpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT PASpatialProp.I FROM ecsql.SSpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PASpatialProp FROM ecsql.SSpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PASpatialProp.Geometry FROM ecsql.SSpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PASpatialProp.Geometry_Array FROM ecsql.SSpatial";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::DateTimeTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55.123'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2013-02-18T06:00:00.000'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports the date and time component delimiter from both SQL-99 (space) and ISO 8601 ('T').", 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = DATE '2012-01-18'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt > DATE '2012-01-18' AND Dt <= DATE '2014-01-01'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt > DATE '2012-01-18' AND Dt <= TIMESTAMP '2014-01-01 12:00:00'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55.123456'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt > TIMESTAMP '2012-01-18 13:02:55.123456Z'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE DtUtc IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE DtUtc IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);


        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = :utc";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = :unspec";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = :loc";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt > :dateonly";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
        }

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = TIMESTAMP '2013-02-18 06:00:00Z'";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = TIMESTAMP '2013-02-18 06:00:00'";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while RHS is not.");

            {
            ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc = :utc";
            auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
            testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
            }

            {
            ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc = :unspec";
            auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
            testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
            }

            {
            ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc > :dateonly";
            auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
            testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
            }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = DtUnspec";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while DtUnspec has kind Unspecified.");

    //Dt has no DateTimeInfo, so it accepts any date time kind on the other side of the expression
    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = Dt";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUnspec has DateTimeKind Unspecified while RHS has DateTimeKind UTC.");

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec = :utc";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec = :unspec";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec > :dateonly";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    {
    ecsql = "SELECT I FROM ecsql.P WHERE Dt = ?";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding string to DateTime parameter is invalid.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
    }

    {
    ecsql = "SELECT I FROM ecsql.P WHERE Dt = ?";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding double to DateTime parameter is invalid.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (2341234.1)));
    }


    //Date-onlys (DateTime::Component::Date) can have any date time kind on other side of operation
    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = Dt";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = DtUtc";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly <= DtUtc";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly < DtUtc";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00', DATE '2013-02-01')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00Z', DATE '2013-02-01')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', DATE '2013-02-01')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :utc)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :unspec)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :loc)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00', :dateonly)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00Z', DATE '2013-02-01')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00Z')";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while first item in list is not.");

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:01Z', :utc)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:01Z', :unspec)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:01Z', :loc)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00Z', :dateonly)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', DATE '2014-02-18')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00Z')";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Dt has date time kind Unspecified, but second item in list has kind Utc.");

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:01', :utc)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:01', :unspec)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:01', :loc)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', :dateonly)";
    auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2014-01-01 00:00:00Z'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND DATE '2014-01-01'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND TIMESTAMP '2014-02-18 06:00:00Z'";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while lower bound operand is not.");

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2014-01-01 00:00:00Z'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND DATE '2014-01-01'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2013-02-01 12:00:00'";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while upper bound operand is not.");

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND DATE '2014-01-01'";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while lower bound operand is not.");

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :utc";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2014, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :unspec";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2014, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :loc";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2014, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :dateonly";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2014, 2, 18)));
        }

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND TIMESTAMP '2014-01-01 00:00:00'";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN TIMESTAMP '2013-02-01T12:00:00' AND DATE '2014-01-01'";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN DATE '2013-02-01' AND TIMESTAMP '2013-02-01 12:00:00Z'";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "DtUnspec has date time kind Unspecified but upper bound has date time kind UTC.");

        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = TIME '13:35:16'";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "TIME constructor (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = TIME '13:35:16.123456'";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "TIME constructor (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = LOCALTIME";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ? OR Dt = ? OR S = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DateTime (DateTime::Kind::Unspecified, 2012, 12, 12, 0, 0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("Sample string")));
        }


    //CURRENT_XXX functions
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_DATE";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, CURRENT_DATE FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_TIMESTAMP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc = CURRENT_TIMESTAMP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec = CURRENT_TIMESTAMP";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "In ECSQL CURRENT_TIMESTAMP returns a UTC time stamp.");

    ecsql = "SELECT I, CURRENT_TIMESTAMP FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_TIME";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    ecsql = "SELECT I, CURRENT_TIME FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::ECInstanceIdTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA a WHERE a.ECInstanceId >= 0";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 AND I < 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 OR I > 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE ECInstanceId IN (I-31, I-32.0)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 2);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE ECInstanceId = '98'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, 1);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE ECInstanceId <= '10000'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE ECInstanceId IN ('91', '92')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, 2);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE ECInstanceId BETWEEN '91' AND '94'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, 4);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE ECInstanceId IN (91, (select ECInstanceId from ecsql.P where ECInstanceId = 92))";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 2);;

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE ECInstanceId = :id";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("id", ECValue (91LL)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE ECInstanceId = :id";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 2, 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("id", ECValue ("91")));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE ECInstanceId = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (92LL)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE ECInstanceId = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 2, 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("91")));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE ECInstanceId = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding string to ECInstanceId parameter is no error as ECInstanceId can be expressed as number and string.", 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("123")));
        }

    ecsql = "SELECT L FROM ecsql.PSA ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "ECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA [ECInstanceId]";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT L AS ECInstanceId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "ECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L AS [ECInstanceId] FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT L FROM ecsql.PSA SourceECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "SourceECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA [SourceECInstanceId]";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT L AS SourceECInstanceId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "SourceECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L AS [SourceECInstanceId] FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT L FROM ecsql.PSA TargetECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "TargetECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA [TargetECInstanceId]";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT L AS TargetECInstanceId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "TargetECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L AS [TargetECInstanceId] FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::FromTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    // select from relationship classes
    Utf8CP ecsql = "SELECT * FROM ONLY ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT * FROM ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ONLY ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT * FROM ONLY ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT * FROM ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ONLY ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    //*******************************************************
    //select from structs 
    //*******************************************************
    //domain class struct
    ecsql = "SELECT ECInstanceId FROM ecsql.SAStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ONLY ecsql.SAStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    //non-domain class struct
    ecsql = "SELECT i, s FROM ecsql.PStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 2, 0);

    ecsql = "SELECT i, s FROM ONLY ecsql.PStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 2, 0);

    ecsql = "SELECT * FROM ecsql.PStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 11, 0);

    ecsql = "SELECT * FROM ONLY ecsql.PStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 11, 0);

    //*******************************************************
    //select from CAs
    //*******************************************************
    ecsql = "SELECT * FROM bsca.DateTimeInfo";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Custom Attributes which are no domain classes are invalid in FROM clause.");

    ecsql = "SELECT * FROM ONLY bsca.DateTimeInfo";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Custom Attributes which are no domain classes are invalid in FROM clause.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    ecsql = "SELECT I, S FROM ecsql.Abstract";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 2*rowCountPerClass); //Abstract class has 2 subclasses, so double row count expected

    ecsql = "SELECT I, S FROM ONLY ecsql.Abstract";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ONLY ecsql.AbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "SELECT I, L FROM ecsql.PUnmapped";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in FROM clause.");

    ecsql = "SELECT I, L FROM ONLY ecsql.PUnmapped";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in FROM clause.");

    //*******************************************************
    // Unsupported classes
    //*******************************************************
    ecsql = "SELECT ECInstanceId FROM bsm.AnyClass";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM ONLY bsm.AnyClass";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM bsm.InstanceCount";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM ONLY bsm.InstanceCount";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "SELECT I, L FROM PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema prefix.");

    ecsql = "SELECT I, L FROM ecsql.BlaBla";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, L FROM blabla.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, blabla FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::FunctionTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT count(*) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT p.count(*) FROM ecsql.PSA p";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Class alias not allowed with count function.");

    ecsql = "SELECT count(NULL) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT p.count(NULL) FROM ecsql.PSA p";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Class alias not allowed with count function.");

    ecsql = "SELECT count(ECInstanceId) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT count(I) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT AVG (I) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT LENGTH (S) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM ecsql.PSA WHERE LENGTH (S) = 0";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, 0);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA WHERE I = ROUND (122.8)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    //**** GetECClassId
    ecsql = "SELECT GetECClassId() FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT GetECClassId( ) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT GetECClassId () FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT GetECClassId ( ) FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT p.GetECClassId() FROM ecsql.PSA p";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT p.GetECClassId(), c.GetECClassId() FROM ecsql.PSA p JOIN ecsql.P c USING ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p WHERE GetECClassId () < 1000";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p WHERE p.GetECClassId () < 1000";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p ORDER BY GetECClassId ()";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p ORDER BY p.GetECClassId ()";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    //invalid expressions
    ecsql = "SELECT p.GetECClassId FROM ecsql.PSA p";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT GetECClassId() FROM ecsql.PSA p JOIN ecsql.P c USING ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT a.GetECClassId() FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT a.GetECClassId() FROM ecsql.PSA p";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::GroupByTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY I";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT B, count(*) FROM ecsql.PSA GROUP BY B";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Bi, count(*) FROM ecsql.PSA GROUP BY Bi";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Hex(Bi), count(*) FROM ecsql.PSA GROUP BY Bi";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT DtUtc, count(*) FROM ecsql.PSA GROUP BY DtUtc";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Geometry, count(*) FROM ecsql.PASpatial GROUP BY Geometry";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    //group by column not in select clause is supported (although against standard)
    ecsql = "SELECT count(*) FROM ecsql.PSA GROUP BY S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, 1);

    //functions in group by is supported (although against standard)
    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY Length(S)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT count(*) FROM ecsql.PSA GROUP BY Length(S)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, 1);

    ecsql = "SELECT Bi, count(*) FROM ecsql.PSA GROUP BY Hex(Bi)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT I, L, count(*) FROM ecsql.PSA GROUP BY I + L";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 3, 1);

    ecsql = "SELECT count(*) FROM ecsql.THBase GROUP BY GetECClassId()";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 1, 6);

    ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY NULL";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY 1";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
    
    ecsql = "SELECT P2D, count(*) FROM ecsql.PSA GROUP BY P2D";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT P3D, count(*) FROM ecsql.PSA GROUP BY P3D";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStructProp, count(*) FROM ecsql.PSA GROUP BY PStructProp";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT Bi_Array, count(*) FROM ecsql.PSA GROUP BY Bi_Array";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStruct_Array, count(*) FROM ecsql.PSA GROUP BY PStruct_Array";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY S HAVING Length(S) > 1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Hex(Bi), count(*) FROM ecsql.P GROUP BY Bi HAVING Hex(Bi) like '0C0B%'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Hex(Bi), count(*) FROM ecsql.P GROUP BY Bi HAVING Hex(Bi) like '1C0B%'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA HAVING Length(S) > 1";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Although standard SQL allows, SQLite doesn't support HAVING without GROUP BY.");

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::InOperatorTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (1, 2, 3)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT IN (1, 2, 3)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (L, I)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT IN (L, I)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE S IN ('hello', 'Sample string')";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (1, ?, 3)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (?)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE S NOT IN ('hello', 'world' )";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE I IN (1, 2, ROUND (122.9879))";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE L IN (1, AVG(L))";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE L IN SELECT L FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE IN (1, 2, 3)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::JoinTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    //JOIN USING
    Utf8CP ecsql = "select ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "select parent.ECInstanceId, child.ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select parent.ECInstanceId, child.ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.*, P.* FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 36, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT end1.I, end2.I FROM ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA FORWARD WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //ambiguous properties in select clause
    ecsql = "SELECT I, L FROM ONLY ecsql.PSA JOIN ONLY ecsql.PSA USING ecsql.PSAHasPSA REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //ambiguous properties in select clause
    ecsql = "SELECT I, L FROM ONLY ecsql.PSA JOIN ONLY ecsql.P USING ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //JOIN ON
    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 "
        "INNER JOIN ONLY ecsql.PSAHasPSA rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "INNER JOIN ONLY ecsql.PSA end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 "
        "INNER JOIN ONLY ecsql.PSAHasP rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "INNER JOIN ONLY ecsql.P end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 "
        "JOIN ONLY ecsql.PSAHasPSA rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "JOIN ONLY ecsql.PSA end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 "
        "JOIN ONLY ecsql.PSAHasP rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "JOIN ONLY ecsql.P end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP REVERSE";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "select * FROM ecsql.PSA INNER JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId INNER JOIN ecsql.P ON P.ECInstanceId = PSAHasP.TargetECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 41, 0);

    //RIGHT JOIN
    ecsql = "select * FROM ecsql.PSA RIGHT JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    //LEFT JOIN not a good example
    ecsql = "select * FROM ecsql.PSA LEFT JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 28, rowCountPerClass);

    ecsql = "select * FROM ecsql.PSAHasPSA_1N";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select * FROM ecsql.PSAHasPSA_11";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select * FROM ecsql.PSAHasPSA_NN";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 10, 0);

    ecsql = "select * FROM ecsql.PHasSA_11P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 16, 0);

    ecsql = "select * FROM ecsql.PHasP_1NPSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 27, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PSAHasPSA_1N REL ORDER BY REL.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PSAHasPSA_11 REL ORDER BY REL.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PSAHasPSA_NN REL ORDER BY REL.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 10, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PHasSA_11P REL ORDER BY REL.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 16, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PHasP_1NPSA REL ORDER BY REL.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 27, 0);

    ecsql = "select * FROM ecsql.PHasP_1NPSA, ecsql.PSAHasPSA_1N, ecsql.PSAHasPSA_11, ecsql.PSAHasPSA_NN, ecsql.PHasSA_11P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 63, 0);

    ecsql = "select COUNT (*) FROM ONLY ecsql.PHasP_1NPSA, ONLY ecsql.PSAHasPSA_1N, ONLY ecsql.PSAHasPSA_11, ONLY ecsql.PSAHasPSA_NN, ONLY ecsql.PHasSA_11P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "select REL.* FROM ecsql.PSA JOIN ecsql.PSA USING ecsql.PSAHasPSA_1N FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "select * FROM ecsql.PSA JOIN ecsql.PSA child USING ecsql.PSAHasPSA_1N FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 51, 0);

    ecsql = "select PSAHasPSA_1N.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_1N RelationShipAliasNotAllowedYet FORWARD";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "select PSAHasPSA_1N.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_1N FORWARD ORDER BY PSAHasPSA_1N.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 51, 0);

    ecsql = "select  PSAHasPSA_11.*, PARENT.*, CHILD.*  FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_11 REVERSE ORDER BY PSAHasPSA_11.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 51, 0);

    ecsql = "select PSAHasPSA_NN.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN  ecsql.PSA CHILD USING ecsql.PSAHasPSA_NN FORWARD ORDER BY PSAHasPSA_NN.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 56, 0);

    ecsql = "select PHasSA_11P.*, P.*, SA.* FROM ONLY ecsql.P JOIN ecsql.SA USING ecsql.PHasSA_11P ORDER BY PHasSA_11P.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 32, 0);

    ecsql = "select PHasP_1NPSA.*, PARENT.*, CHILD.* FROM ecsql.P PARENT JOIN ecsql.P CHILD USING ecsql.PHasP_1NPSA REVERSE ORDER BY PHasP_1NPSA.ECInstanceId DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 53, 0);

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::LikeOperatorTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam%'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I LIKE 'Sam%'";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 10";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE NULL";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam_le string'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam%' ESCAPE '\\'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);


    //escaping the wild card % should not return any rows
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam\\%' ESCAPE '\\'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    //escaping the wild card _ should not return any rows
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam\\_le string' ESCAPE '\\'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    //invalid escape clauses
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE 10";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE S";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam\\%' {ESCAPE '\\'}";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("Samp%")));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        //bind null
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S NOT LIKE ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        //bind null. In SQLite LIKE NULL or NOT LIKE NULL is always false, therefore no rows expected.
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding non-string primitive value to parameter in LIKE expression is expected to work as SQLite converts the parameter value implicitly.", 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.0, 1.0, 1.0))));
        }

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::LimitTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass, 5));

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 1+1+1+2";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass, 5));


    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5 OFFSET 3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass + 3, 5));

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 10/2 OFFSET 3*1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass + 3, 5));

        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 2);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (2)));
        }

        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5 - ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 3);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (2)));
        }


        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5 OFFSET ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - 3, 5));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3)));
        }

        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT ? OFFSET ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - 3, 5));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (5)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3)));
        }

        {
        int pagesize = 3;
        int pageno = 2;
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT :pagesize OFFSET :pagesize * :pageno";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - (pagesize * pageno), pagesize));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pagesize", ECValue (pagesize)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pageno", ECValue (pageno)));
        }

        {
        int pagesize = 3;
        int pageno = 2;
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT :pagesize OFFSET :pagesize * (:pageno - 1)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - (pagesize * (pageno - 1)), pagesize));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pagesize", ECValue (pagesize)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pageno", ECValue (pageno)));
        }

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 10 OFFSET";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::MiscTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    // Statements where non-optional clauses are missing
    //*******************************************************
    Utf8CP ecsql = "";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "FROM ONLY ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT FROM ONLY ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S WHERE L > 109222";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Select clause
    //*******************************************************
    
    ecsql = "SELECT * FROM ecsql.PSA a";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 23, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 13, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.P WHERE ECInstanceId >= 0";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 13, rowCountPerClass);

    ecsql = "SELECT a.* FROM ecsql.P a WHERE a.ECInstanceId >= 0";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 13, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.SA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.SAStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);


    //*******************************************************
    // Special tokens
    //*******************************************************
    //These were reserved words in the original grammar introduced by some ODBC data time functions.
    //The ODBC stuff was removed from the ECSQL grammar, and the following tests serve as safeguards
    //against regressions when updating the grammar.

    ecsql = "SELECT D FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, S FROM ecsql.PSA d";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA t";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA ts";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA Z";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //*******************************************************
    // Select clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    ecsql = "SELECT _A_B_C,_ABC,_ABC_,A_B_C_,ABC_ FROM ecsql._UnderBar";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 5, rowCountPerClass);

    ecsql = "SELECT [_A_B_C],[_ABC],[_ABC_],[A_B_C_],[ABC_] FROM ecsql.[_UnderBar]";
    ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 5, rowCountPerClass);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::NullLiteralTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "select NULL FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "select NULL, I FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select NULL as I FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "select NULL as P3D FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "select NULL as StructProp FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "select NULL as PStruct_Array FROM ecsql.SA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NULL OR I IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NULL AND I IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS NOT 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL <> 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Comparing NULL with non-NULL values are always false", 3, 0);

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I IS NOT ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ())); //bind null value
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I IS ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ())); //bind null value
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE ? IS NULL";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE ? = NULL";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE ? <> NULL";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::OrderByTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I FROM ecsql.PSA ORDER BY L";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L ASC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L, S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L ASC, S DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY Dt";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY Dt ASC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY Dt DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY DtUtc DESC";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 ORDER BY ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA a WHERE a.ECInstanceId >= 0 ORDER BY a.ECInstanceId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY S";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY LOWER (S)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY UPPER (S)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    //constant value exp as order by -> no-op
    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY 1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    //boolean exp as order by
    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY I < 123";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY P2D DESC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY Point2D is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA tt ORDER BY tt.P2D ASC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY Point2D is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY P3D DESC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY Point3D is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY PStructProp DESC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY ECStruct is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY I_Array ASC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY arrays is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY P2D_Array DESC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY arrays is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY PStruct_Array ASC";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "ORDER BY arrays is not supported by ECSQL.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::ParameterAdvancedTests (int rowCountPerClass)
    {
    //This includes only advanced parameter tests that are not covered implicitly by the other test datasets

    ECSqlTestDataset dataset;

        {
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = :i AND S = :s AND L = :i * 1000000 + 456789";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("i", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("Sample string")));
        }

        {
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("Sample string")));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("l", ECValue (123456789LL)));
        }

        {
        //use unary operator (-) with parameter
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = -?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (-123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(-123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and don't bind anything to it
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = ?";
        ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0); //NULL = NULL is false
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and don't bind anything to it
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?"; //NULL = -NULL is not true
        ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0);
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and only bind to one parameter
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE :p1 > -:p1";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue("p1", ECValue(123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and don't bind anything to it
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE :p1 > -:p1";
        ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0);
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE :p1 = -:p1";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, 0);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue("p1", ECValue(123)));
        }

        {
        Utf8CP ecsql = "SELECT ?, S FROM ecsql.PSA";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

        {
        Utf8CP ecsql = "SELECT ? AS NewProp, S FROM ecsql.PSA";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::PointTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT P2D FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT P3D FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA a";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D = P2D";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D <> P2D";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D IN (P2D, P2D)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D NOT IN (P2D, P2D)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D >= P2D";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D <= P2D";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D BETWEEN P2D AND P2D";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D >= ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D >= ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding int to Point3D parameter is invalid.");

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (0.0, 1.0))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA a WHERE a.P2D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA a WHERE a.P2D <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        //!= NULL doesn't return any records even if LHS is not null
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D != ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        {
        ecsql = "SELECT I FROM ecsql.P WHERE P2D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding DPoint3D to Point2D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.0, 1.0, 1.0))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P2D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding int to Point2D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P2D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding string to Point2D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
        }

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.23, 22.14, 33.12))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.00, 22.00, 33.00))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P3D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding DPoint2D to Point3D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.0, 1.0))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P3D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding int to Point3D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P3D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding string to Point3D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
        }

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D = P3D";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D = P2D";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D IS NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D IS NOT NULL";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (-1.3, 45.134)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (0, 0)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (0)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (-1.3)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (-1.3, 45.134, 10)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (0, 0, 0)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (0, 0)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (1, -34.1)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (P2D, P2D)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D IN (P3D, P3D)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D NOT IN (P2D, P2D)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D NOT IN (P3D, P3D)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (?, ?)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (:p1, :p2)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR (P2D = :p2 AND P2D != :p1)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR P2D = :p2 OR P2D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint2d::From (-1.0, -2.0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR (P2D != :p1 AND B = ?)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (true)));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P3D IN (?, ?)";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.23, 22.14, 33.12))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P3D = :p1 OR P3D = :p2 OR P3D = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint3d::From (0.0, 0.0, 0.0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint3d::From (1.0, 1.0, 1.0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.23, 22.14, 33.12))));
        }

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (POINT2D (1,1), POINT2D (2,2))";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D IN (POINT3D (1,1,1), POINT3D (2,2,2))";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D BETWEEN POINT2D (1,1) AND POINT2D (2,2)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D BETWEEN POINT3D (0,0,0) AND POINT3D (10,10,10)";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported);


    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::PolymorphicTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, L FROM ONLY ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //Do non-polymorphic query on TablePerHierarchy    
    ecsql = "SELECT * FROM ONLY ecsql.THBase";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH2";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH4";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH5";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass);

    //Do polymorphic query on TablePerHierarchy    
    ecsql = "SELECT * FROM ecsql.THBase";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass * 6);

    ecsql = "SELECT * FROM ecsql.TH1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass * 5);

    ecsql = "SELECT * FROM ecsql.TH2";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass * 4);

    ecsql = "SELECT * FROM ecsql.TH3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass * 3);

    ecsql = "SELECT * FROM ecsql.TH4";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass * 2);

    ecsql = "SELECT * FROM ecsql.TH5";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass * 1);

    //Do non-polymorphic query on TablePerClass    
    ecsql = "SELECT * FROM ONLY ecsql.TCBase";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC2";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC4";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC5";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass);

    //Do polymorphic query on TablePerClass    
    ecsql = "SELECT * FROM ecsql.TCBase";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass * 6);

    ecsql = "SELECT * FROM ecsql.TC1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass * 5);

    ecsql = "SELECT * FROM ecsql.TC2";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass * 4);

    ecsql = "SELECT * FROM ecsql.TC3";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass * 3);

    ecsql = "SELECT * FROM ecsql.TC4";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass * 2);

    ecsql = "SELECT * FROM ecsql.TC5";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass * 1);

    ecsql = "select ECInstanceId FROM ecsql.Empty";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "select ECInstanceId, I, S from ecsql.AbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "select ECInstanceId from ecsql.EmptyAbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "select ECInstanceId, I, S from only ecsql.AbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "select ECInstanceId from only ecsql.EmptyAbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::PrimitiveTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, L FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, L FROM ecsql.PSA a";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT a.I, a.L FROM ecsql.PSA a";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT B, ECInstanceId, S FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT Bi FROM ecsql.PSA a";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT 3.14 FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT 3.14 FROM ecsql.PSA WHERE L = 0";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "SELECT 1000 AS I FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT 3.14 AS BlaBla FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (12)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = :p";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p", ECValue (123)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = :p";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p", ECValue (12)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = :p1 OR I = :p2";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (12LL)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (123)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = :p1 OR I = :p2";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (12LL)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = ? OR I = :p1 OR B = :p2 OR S = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (100LL)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (true)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("blabla")));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = ? OR I = :p1 OR I > :p1 OR S = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (100LL)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (120)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("blabla")));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding string to integer parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("blabla")));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding double to integer parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3.14)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding boolean to integer parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (true)));
        }


        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding DPointXD to int parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.0, 1.0))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE D <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Binding DateTime to double parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 9, 12)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE D > ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding long to double parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1LL)));
        }


    //***** Boolean properties
    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = true";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = True";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = false";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = False";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = Unknown";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = UNKNOWN";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        {
        ecsql = "SELECT I FROM ecsql.P WHERE B = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding long to boolean parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1LL)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE B <> ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, IECSqlExpectedResult::Category::Supported, "Binding long to boolean parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
        }

    //***** String literals
    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S = 'Sample string'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S = 'Sample \"string'";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S = \"Sample string\"";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "String literals must be surrounded by single quotes. Double quotes are equivalent to square brackets in SQL.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::SourceTargetConstraintTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT SourceECClassId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT GetECClassId (), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.PSAHasP_N1";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT TargetECClassId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //link table mapping
    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasPSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    //end table mapping
    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP WHERE SourceECClassId = TargetECClassId AND GetECClassId () = 180";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT rel.GetECClassId (), rel.SourceECClassId, rel.TargetECClassId FROM ecsql.PSAHasP rel WHERE rel.SourceECClassId = rel.TargetECClassId AND rel.GetECClassId () = 180";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP ORDER BY GetECClassId (), SourceECClassId, TargetECClassId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP rel ORDER BY rel.GetECClassId (), rel.SourceECClassId, rel.TargetECClassId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT rel.GetECClassId (), rel.SourceECClassId, rel.TargetECClassId FROM ecsql.PSAHasP rel";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 0);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::StructTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT PStructProp.i, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    //Struct member property I does not exist
    ecsql = "SELECT PStructProp.I, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStructProp FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT B, PStructProp FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT B, PStructProp, P3D FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT SAStructProp FROM ecsql.SA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT SAStructProp.PStructProp FROM ecsql.SA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT SAStructProp.PStructProp, SAStructProp.PStructProp.p3d FROM ecsql.SA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp = ?";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i = ?";
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        ecsql = "SELECT I, S FROM ecsql.PSA tt WHERE tt.PStructProp = ?";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp IS NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp IS NOT NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NULL";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 0);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NOT NULL";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp IS NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp IS NOT NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp.PStructProp IS NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp.PStructProp IS NOT NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Structs are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE PStruct_Array IS NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Struct arrays are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE PStruct_Array IS NOT NULL";
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Struct arrays are not supported in where clause.");

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NOT NULL";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IN (10, 123, 200)";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i BETWEEN 10 AND 200";
        ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::SubqueryTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT ECInstanceId FROM ecsql.P WHERE ECInstanceId < (SELECT avg(ECInstanceId) FROM ecsql.P)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass/2);

    ecsql = "SELECT ECInstanceId FROM (SELECT * FROM ecsql.P)";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::UnionTests(int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT ECInstanceId FROM ecsql.P UNION SELECT ECInstanceId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass*2);;

    ecsql = "SELECT ECInstanceId FROM ecsql.P UNION ALL SELECT ECInstanceId FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass*2);

    ecsql = "SELECT B, Bi, I, L, S, P2D, P3D FROM ecsql.P UNION ALL SELECT B, Bi, I, L, S, P2D, P3D FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 7, rowCountPerClass*2);

    ecsql = "SELECT PStructProp FROM ecsql.PSA UNION SELECT PStructProp FROM ecsql.SAStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT B_Array, Bi_Array, D_Array, Dt_Array, I_Array, S_Array, P2D_Array, P3D_Array FROM ecsql.PSA UNION ALL SELECT B_Array, Bi_Array, D_Array, Dt_Array, I_Array, S_Array, P2D_Array, P3D_Array FROM ecsql.PA";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 8, rowCountPerClass*2);

    ecsql = "SELECT PStruct_Array FROM ecsql.PSA UNION SELECT PStruct_Array FROM ecsql.SAStruct";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1,  20);

    ecsql = "SELECT ECClassId, COUNT(*) FROM (SELECT GetECClassId() ECClassId, ECInstanceId FROM ecsql.PSA UNION ALL SELECT GetECClassId() ECClassId, ECInstanceId FROM ecsql.SAStruct) GROUP BY ECClassId";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 2, 2);

    ecsql = "SELECT ECClassId, Name, COUNT(*) FROM (SELECT GetECClassId() ECClassId, ECInstanceId, 'PSA' Name FROM ecsql.PSA UNION ALL SELECT GetECClassId() ECClassId, ECInstanceId, 'SAStruct' Name FROM ecsql.SAStruct) GROUP BY ECClassId, Name";
    ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 3, 2);


    ecsql = "SELECT S FROM ecsql.P UNION SELECT Dt FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT Bi FROM ecsql.P UNION SELECT I_Array FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT Bi_Array FROM ecsql.PSA UNION SELECT I_Array FROM ecsql.PA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT * FROM ecsql.P UNION SELECT * FROM ecsql.PSA";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing(dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

END_ECDBUNITTESTS_NAMESPACE
