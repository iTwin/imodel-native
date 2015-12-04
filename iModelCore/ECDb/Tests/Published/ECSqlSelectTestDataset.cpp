/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlSelectTestDataset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlSelectTestDataset.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::AliasTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;
    Utf8CP ecsql = "SELECT ECInstanceId, PStructProp A11 FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2);

    //tests when class alias is same as a property name. This should work unless the property is a struct property
    ecsql = "SELECT S.ECInstanceId FROM ecsql.PSA S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM ecsql.PSA S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM ecsql.PSA S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.I FROM ecsql.PSA S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM (SELECT S FROM ecsql.PSA) S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM (SELECT S FROM ecsql.PSA) S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM (SELECT S, I FROM ecsql.PSA) S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.I FROM (SELECT S, I FROM ecsql.PSA) S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM ecsql.PSA, (SELECT ECInstanceId, I, S FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.Bla FROM ecsql.PSA, (SELECT ECInstanceId, I, 3.14 AS Bla FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S.S FROM ecsql.PSA, (SELECT I, S FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStructProp FROM ecsql.PSA PStructProp";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.ECInstanceId FROM ecsql.PSA PStructProp";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.s FROM ecsql.PSA PStructProp";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.I FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.PStructProp FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT PStructProp.i FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::ArrayTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT Dt_Array, B FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array = ?";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array <> ?";
    //unbound parameters mean NULL and comparing NULL with NULL is always false
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array IS NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE Dt_Array IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE CARDINALITY(Dt_Array) > 0";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT S, PStruct_Array FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStruct_Array = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported, "Struct arrays are not supported yet in where clause.");

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStruct_Array IS NULL";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported, "Struct arrays are not supported yet in where clause.");

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStruct_Array IS NOT NULL";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported, "Struct arrays are not supported yet in where clause.");
    
    ecsql = "SELECT I, S FROM ecsql.PSA WHERE CARDINALITY(PStruct_Array) > 0";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT Dt_Array[1], B FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT Dt_Array[100000], B FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT Dt_Array[-1], B FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT UnknowProperty[1], B FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT UnknowProperty[-1], B FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::BetweenOperatorTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I BETWEEN 1 AND 3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I BETWEEN 122 AND 124";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN 1 AND 3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    //S always amounts to Sample String
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S BETWEEN 'Q' AND 'T'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    //S always amounts to Sample String
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S BETWEEN 'Q' AND 'R'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN ? AND 3";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN 1 AND ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN ? AND ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
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
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS BINARY) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS BINARY) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (PStructProp AS BINARY) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS BINARY) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
   

    ecsql = "SELECT CAST (B AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (1 AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (I AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (True AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (False AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST ('1' AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (Unknown AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "SQL-99 keyword UNKNOWN not supported in ECSQL as ECObjects doesn't have a counterpart for it.");

    ecsql = "SELECT CAST (NULL AS BOOLEAN) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (B AS BOOL) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Only keyword BOOLEAN supported.");

    ecsql = "SELECT CAST (Bi AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (Dt AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS DATETIME) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DtUtc AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DtUtc AS DATETIME) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DtUtc AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (TIMESTAMP '2013-02-09T12:00:00' AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (TIMESTAMP '2013-02-09T12:00:00' AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (DATE '2013-02-09' AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (D AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (123425 AS DATETIME) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (123425.2343 AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (True AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (123425.123 AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (S AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P2D AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS TIMESTAMP) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS DATE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    
    ecsql = "SELECT CAST (Bi AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (L AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P2D AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS DOUBLE) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    
    ecsql = "SELECT CAST (Bi AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (False AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS INT32) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    
    ecsql = "SELECT CAST (Bi AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (L AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (L AS INT64) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (True AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS LONG) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


    ecsql = "SELECT CAST (B AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (True AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (False AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Dt AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (S AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (P2D AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (P3D AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS TEXT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "TEXT is SQLite specific datatype which is not supported by SQLite.");

    ecsql = "SELECT CAST (NULL AS STRING) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


    ecsql = "SELECT CAST (Bi AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (I AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (D AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (S AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT CAST (Bi AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (I AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (D AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (S AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);


    ecsql = "SELECT CAST (D AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (3.134 AS POINT2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (L AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (100000123 AS POINT3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);


    ecsql = "SELECT CAST (L AS ecsql.PStruct) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (PStructProp AS ecsql.PStruct) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT CAST (NULL AS ecsql.PStruct) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);


    //Support for geometry is not fleshed out yet. Possible type names are tested here
    ecsql = "SELECT CAST (I AS IGeometry) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (I AS Geometry) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS IGeometry) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (NULL AS Geometry) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);


    ecsql = "SELECT CAST (S AS I) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT CAST (? AS INT) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT CAST (I AS ?) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);


    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::CommonGeometryTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Geometry, S FROM ecsql.PASpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Geometry_Array, S FROM ecsql.PASpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PASpatial ORDER BY Geometry";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Common Geometry properties cannot be ordered by ECSQL");

    ecsql = "SELECT * FROM ecsql.PASpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 7, rowCountPerClass);

    ecsql = "SELECT p.* FROM ecsql.PASpatial p";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 7, rowCountPerClass);

    ecsql = "SELECT count(*) FROM ecsql.PASpatial p";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT * FROM ecsql.SSpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT SpatialStructProp FROM ecsql.SSpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT SpatialStructProp.Geometry FROM ecsql.SSpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT SpatialStructProp.Geometry_Array FROM ecsql.SSpatial";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::DateTimeTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55.123'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2013-02-18T06:00:00.000'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "ECSQL supports the date and time component delimiter from both SQL-99 (space) and ISO 8601 ('T').", 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = DATE '2012-01-18'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt > DATE '2012-01-18' AND Dt <= DATE '2014-01-01'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt > DATE '2012-01-18' AND Dt <= TIMESTAMP '2014-01-01 12:00:00'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55.123456'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt > TIMESTAMP '2012-01-18 13:02:55.123456Z'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt IS NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE Dt IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE DtUtc IS NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE DtUtc IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);


        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = :utc";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = :unspec";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = :loc";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt > :dateonly";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
        }

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = TIMESTAMP '2013-02-18 06:00:00Z'";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = TIMESTAMP '2013-02-18 06:00:00'";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while RHS is not.");

            {
            ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc = :utc";
            auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
            testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
            }

            {
            ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc = :unspec";
            auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
            testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
            }

            {
            ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc > :dateonly";
            auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
            testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
            }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = DtUnspec";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while DtUnspec has kind Unspecified.");

    //Dt has no DateTimeInfo, so it accepts any date time kind on the other side of the expression
    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = Dt";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUnspec has DateTimeKind Unspecified while RHS has DateTimeKind UTC.");

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec = :utc";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec = :unspec";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec > :dateonly";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    {
    ecsql = "SELECT I FROM ecsql.P WHERE Dt = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding string to DateTime parameter is invalid.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
    }

    {
    ecsql = "SELECT I FROM ecsql.P WHERE Dt = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding double to DateTime parameter is invalid.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (2341234.1)));
    }


    //Date-onlys (DateTime::Component::Date) can have any date time kind on other side of operation
    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = Dt";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = DtUtc";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly <= DtUtc";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DateOnly < DtUtc";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00', DATE '2013-02-01')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00Z', DATE '2013-02-01')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', DATE '2013-02-01')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :utc)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :unspec)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :loc)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00', :dateonly)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00Z', DATE '2013-02-01')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while first item in list is not.");

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:01Z', :utc)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:01Z', :unspec)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:01Z', :loc)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00Z', :dateonly)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', DATE '2014-02-18')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Dt has date time kind Unspecified, but second item in list has kind Utc.");

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:01', :utc)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:01', :unspec)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:01', :loc)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', :dateonly)";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2013, 2, 18)));
    }

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2014-01-01 00:00:00Z'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND DATE '2014-01-01'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND TIMESTAMP '2014-02-18 06:00:00Z'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while lower bound operand is not.");

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2014-01-01 00:00:00Z'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND DATE '2014-01-01'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2013-02-01 12:00:00'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while upper bound operand is not.");

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND DATE '2014-01-01'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while lower bound operand is not.");

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :utc";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("utc", DateTime (DateTime::Kind::Utc, 2014, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :unspec";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("unspec", DateTime (DateTime::Kind::Unspecified, 2014, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :loc";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("loc", DateTime (DateTime::Kind::Local, 2014, 2, 18, 6, 0, 0)));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :dateonly";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("dateonly", DateTime (2014, 2, 18)));
        }

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND TIMESTAMP '2014-01-01 00:00:00'";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN TIMESTAMP '2013-02-01T12:00:00' AND DATE '2014-01-01'";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN DATE '2013-02-01' AND TIMESTAMP '2013-02-01 12:00:00Z'";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUnspec has date time kind Unspecified but upper bound has date time kind UTC.");

        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = TIME '13:35:16'";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "TIME constructor (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = TIME '13:35:16.123456'";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "TIME constructor (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = LOCALTIME";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ? OR Dt = ? OR S = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DateTime (DateTime::Kind::Unspecified, 2012, 12, 12, 0, 0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("Sample string")));
        }


    //CURRENT_XXX functions
    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_DATE";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, CURRENT_DATE FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUtc = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE DtUnspec = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "In ECSQL CURRENT_TIMESTAMP returns a UTC time stamp.");

    ecsql = "SELECT I, CURRENT_TIMESTAMP FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_TIME";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    ecsql = "SELECT I, CURRENT_TIME FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::ECInstanceIdTests (ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    bvector<ECInstanceId> psaIds;

            {
            ECSqlStatement stmt;
            if (ECSqlStatus::Success != stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ONLY ecsql.PSA"))
                return dataset;

            while (stmt.Step() == BE_SQLITE_ROW)
                psaIds.push_back(stmt.GetValueId<ECInstanceId>(0));
            }

    if (rowCountPerClass != (int) psaIds.size() || rowCountPerClass < 5)
        {
        LOG.errorv("Test set up failure. Row count per class (%d) should match the number of ids in test class (%d) and should be at least 5.",
                    rowCountPerClass, psaIds.size());
        BeAssert(false);
        return dataset;
        }

    Utf8CP ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA a WHERE a.ECInstanceId >= 0";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 AND I < 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 OR I > 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId ='%lld'", psaIds[0].GetValue());
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, 1);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId <= '10000'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, rowCountPerClass);

    ecsqlStr.Sprintf("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId IN ('%lld', '%lld')", psaIds[0].GetValue(), psaIds[2].GetValue());
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, 2);

    ecsqlStr.Sprintf("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId BETWEEN '%lld' AND '%lld'", psaIds[0].GetValue(), psaIds[3].GetValue());
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 3, 4);

    ecsqlStr.Sprintf("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId IN (%lld, (select ECInstanceId from ecsql.PSA where ECInstanceId = %lld))", psaIds[0].GetValue(), psaIds[2].GetValue());
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsqlStr.c_str(), 3, 2);;

        {
        ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE ECInstanceId = :id";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("id", ECValue ((int64_t)psaIds[0].GetValue())));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE ECInstanceId = :id";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 2, 1);
        Utf8String strVal;
        strVal.Sprintf("%lld", psaIds[0].GetValue());
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("id", ECValue (strVal.c_str())));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE ECInstanceId = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 1);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ((int64_t)psaIds[0].GetValue())));
        }

        {
        ecsql = "SELECT I, Dt FROM ecsql.PSA WHERE ECInstanceId = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "ECSQL supports implicit conversion from string to number for ECInstanceId.", 2, 1);
        Utf8String strVal;
        strVal.Sprintf("%lld", psaIds[0].GetValue());
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(strVal.c_str())));
        }

    ecsql = "SELECT L FROM ecsql.PSA ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "ECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA [ECInstanceId]";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT L AS ECInstanceId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "ECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L AS [ECInstanceId] FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT L FROM ecsql.PSA SourceECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "SourceECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA [SourceECInstanceId]";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT L AS SourceECInstanceId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "SourceECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L AS [SourceECInstanceId] FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT L FROM ecsql.PSA TargetECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "TargetECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA [TargetECInstanceId]";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT L AS TargetECInstanceId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "TargetECInstanceId might become a reserved word.", 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L AS [TargetECInstanceId] FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

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
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT * FROM ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ONLY ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT * FROM ONLY ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT * FROM ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ONLY ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    //*******************************************************
    //select from structs 
    //*******************************************************
    ecsql = "SELECT i, s FROM ecsql.PStruct";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 2, 0);

    ecsql = "SELECT i, s FROM ONLY ecsql.PStruct";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 2, 0);

    ecsql = "SELECT * FROM ecsql.PStruct";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 11, 0);

    ecsql = "SELECT * FROM ONLY ecsql.PStruct";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::SupportedButMightBecomeInvalid, "Querying from an ECStruct which is not a domain class is invalid and will be unsupported in the future.", 11, 0);

    //*******************************************************
    //select from CAs
    //*******************************************************
    ecsql = "SELECT * FROM bsca.DateTimeInfo";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Custom Attributes which are no domain classes are invalid in FROM clause.");

    ecsql = "SELECT * FROM ONLY bsca.DateTimeInfo";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Custom Attributes which are no domain classes are invalid in FROM clause.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    ecsql = "SELECT I, S FROM ecsql.Abstract";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 2*rowCountPerClass); //Abstract class has 2 subclasses, so double row count expected

    ecsql = "SELECT I, S FROM ONLY ecsql.Abstract";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ONLY ecsql.AbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.AbstractR";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported, "THis should work and will be fixed.");

    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "SELECT I, L FROM ecsql.PUnmapped";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in FROM clause.");

    ecsql = "SELECT I, L FROM ONLY ecsql.PUnmapped";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in FROM clause.");

    //*******************************************************
    // Unsupported classes
    //*******************************************************
    ecsql = "SELECT ECInstanceId FROM bsm.AnyClass";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM ONLY bsm.AnyClass";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM bsm.InstanceCount";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM ONLY bsm.InstanceCount";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "SELECT I, L FROM PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema prefix.");

    ecsql = "SELECT I, L FROM ecsql.BlaBla";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, L FROM blabla.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, blabla FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::FunctionTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT count(*) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT p.count(*) FROM ecsql.PSA p";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class alias not allowed with count function.");

    ecsql = "SELECT count(NULL) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT p.count(NULL) FROM ecsql.PSA p";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class alias not allowed with count function.");

    ecsql = "SELECT count(ECInstanceId) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT count(I) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT AVG (I) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT LENGTH (S) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT S FROM ecsql.PSA WHERE LENGTH (S) = 0";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 0);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA WHERE I = ROUND (122.8)";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    //**** GetECClassId
    ecsql = "SELECT GetECClassId() FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT GetECClassId( ) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT GetECClassId () FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT GetECClassId ( ) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT p.GetECClassId() FROM ecsql.PSA p";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT p.GetECClassId(), c.GetECClassId() FROM ecsql.PSA p JOIN ecsql.P c USING ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p WHERE GetECClassId () < 1000";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p WHERE p.GetECClassId () < 1000";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p ORDER BY GetECClassId ()";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM ecsql.PSA p ORDER BY p.GetECClassId ()";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    //invalid expressions
    ecsql = "SELECT p.GetECClassId FROM ecsql.PSA p";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT GetECClassId() FROM ecsql.PSA p JOIN ecsql.P c USING ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT a.GetECClassId() FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT a.GetECClassId() FROM ecsql.PSA p";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::GroupByTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY I";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT B, count(*) FROM ecsql.PSA GROUP BY B";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Bi, count(*) FROM ecsql.PSA GROUP BY Bi";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Hex(Bi), count(*) FROM ecsql.PSA GROUP BY Bi";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT DtUtc, count(*) FROM ecsql.PSA GROUP BY DtUtc";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Geometry, count(*) FROM ecsql.PASpatial GROUP BY Geometry";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //group by column not in select clause is supported (although against standard)
    ecsql = "SELECT count(*) FROM ecsql.PSA GROUP BY S";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 1);

    //functions in group by is supported (although against standard)
    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY Length(S)";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT count(*) FROM ecsql.PSA GROUP BY Length(S)";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 1);

    ecsql = "SELECT Bi, count(*) FROM ecsql.PSA GROUP BY Hex(Bi)";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT I, L, count(*) FROM ecsql.PSA GROUP BY I + L";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 1);

    ecsql = "SELECT count(*) FROM ecsql.THBase GROUP BY GetECClassId()";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 6);

    ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY NULL";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, count(*) FROM ecsql.PSA GROUP BY 1";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    
    ecsql = "SELECT P2D, count(*) FROM ecsql.PSA GROUP BY P2D";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT P3D, count(*) FROM ecsql.PSA GROUP BY P3D";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT PStructProp, count(*) FROM ecsql.PSA GROUP BY PStructProp";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Bi_Array, count(*) FROM ecsql.PSA GROUP BY Bi_Array";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStruct_Array, count(*) FROM ecsql.PSA GROUP BY PStruct_Array";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT Geometry, count(*) FROM ecsql.PASpatial GROUP BY I HAVING Geometry IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY S HAVING PStructProp IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA GROUP BY S HAVING Length(S) > 1";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Hex(Bi), count(*) FROM ecsql.P GROUP BY Bi HAVING Hex(Bi) like '0C0B%'";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 1);

    ecsql = "SELECT Hex(Bi), count(*) FROM ecsql.P GROUP BY Bi HAVING Hex(Bi) like '1C0B%'";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);

    ecsql = "SELECT S, count(*) FROM ecsql.PSA HAVING Length(S) > 1";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Although standard SQL allows, SQLite doesn't support HAVING without GROUP BY.");

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::InOperatorTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (1, 2, 3)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT IN (1, 2, 3)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (L, I)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT IN (L, I)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE S IN ('hello', 'Sample string')";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (1, ?, 3)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I IN ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE S NOT IN ('hello', 'world' )";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE I IN (1, 2, ROUND (122.9879))";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE L IN (1, AVG(L))";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE L IN SELECT L FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, Dt, S FROM ecsql.P WHERE IN (1, 2, 3)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

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
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select parent.ECInstanceId, child.ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA REVERSE";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select parent.ECInstanceId, child.ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA FORWARD";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.*, P.* FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 36, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA FORWARD";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT end1.I, end2.I FROM ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA FORWARD WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA REVERSE WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP FORWARD";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP REVERSE";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //ambiguous properties in select clause
    ecsql = "SELECT I, L FROM ONLY ecsql.PSA JOIN ONLY ecsql.PSA USING ecsql.PSAHasPSA REVERSE";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //ambiguous properties in select clause
    ecsql = "SELECT I, L FROM ONLY ecsql.PSA JOIN ONLY ecsql.P USING ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //JOIN ON
    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 "
        "INNER JOIN ONLY ecsql.PSAHasPSA rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "INNER JOIN ONLY ecsql.PSA end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 "
        "INNER JOIN ONLY ecsql.PSAHasP rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "INNER JOIN ONLY ecsql.P end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 "
        "JOIN ONLY ecsql.PSAHasPSA rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "JOIN ONLY ecsql.PSA end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 "
        "JOIN ONLY ecsql.PSAHasP rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "JOIN ONLY ecsql.P end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP FORWARD";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP REVERSE";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select * FROM ecsql.PSA INNER JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId INNER JOIN ecsql.P ON P.ECInstanceId = PSAHasP.TargetECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 41, 0);

    //RIGHT JOIN
    ecsql = "select * FROM ecsql.PSA RIGHT JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    //LEFT JOIN not a good example
    ecsql = "select * FROM ecsql.PSA LEFT JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 28, rowCountPerClass);

    ecsql = "select * FROM ecsql.PSAHasPSA_1N";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select * FROM ecsql.PSAHasPSA_11";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select * FROM ecsql.PSAHasPSA_NN";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 10, 0);

    ecsql = "select * FROM ecsql.PHasSA_11P";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 16, 0);

    ecsql = "select * FROM ecsql.PHasP_1NPSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 27, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PSAHasPSA_1N REL ORDER BY REL.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PSAHasPSA_11 REL ORDER BY REL.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PSAHasPSA_NN REL ORDER BY REL.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 10, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PHasSA_11P REL ORDER BY REL.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 16, 0);

    ecsql = "select REL.* FROM ONLY ecsql.PHasP_1NPSA REL ORDER BY REL.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 27, 0);

    ecsql = "select * FROM ecsql.PHasP_1NPSA, ecsql.PSAHasPSA_1N, ecsql.PSAHasPSA_11, ecsql.PSAHasPSA_NN, ecsql.PHasSA_11P";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 63, 0);

    ecsql = "select COUNT (*) FROM ONLY ecsql.PHasP_1NPSA, ONLY ecsql.PSAHasPSA_1N, ONLY ecsql.PSAHasPSA_11, ONLY ecsql.PSAHasPSA_NN, ONLY ecsql.PHasSA_11P";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "select REL.* FROM ecsql.PSA JOIN ecsql.PSA USING ecsql.PSAHasPSA_1N FORWARD";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select * FROM ecsql.PSA JOIN ecsql.PSA child USING ecsql.PSAHasPSA_1N FORWARD";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 51, 0);

    ecsql = "select PSAHasPSA_1N.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_1N RelationShipAliasNotAllowedYet FORWARD";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select PSAHasPSA_1N.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_1N FORWARD ORDER BY PSAHasPSA_1N.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 51, 0);

    ecsql = "select  PSAHasPSA_11.*, PARENT.*, CHILD.*  FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_11 REVERSE ORDER BY PSAHasPSA_11.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 51, 0);

    ecsql = "select PSAHasPSA_NN.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN  ecsql.PSA CHILD USING ecsql.PSAHasPSA_NN FORWARD ORDER BY PSAHasPSA_NN.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 56, 0);

    ecsql = "select PHasSA_11P.*, P.*, SA.* FROM ONLY ecsql.P JOIN ecsql.SA USING ecsql.PHasSA_11P ORDER BY PHasSA_11P.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 32, 0);

    ecsql = "select PHasP_1NPSA.*, PARENT.*, CHILD.* FROM ecsql.P PARENT JOIN ecsql.P CHILD USING ecsql.PHasP_1NPSA REVERSE ORDER BY PHasP_1NPSA.ECInstanceId DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 53, 0);

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::LikeOperatorTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam%'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE I LIKE 'Sam%'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 10";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE NULL";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam_le string'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam%' ESCAPE '\\'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);


    //escaping the wild card % should not return any rows
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam\\%' ESCAPE '\\'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    //escaping the wild card _ should not return any rows
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam\\_le string' ESCAPE '\\'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    //invalid escape clauses
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE 10";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE S";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam\\%' {ESCAPE '\\'}";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("Samp%")));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        //bind null
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S NOT LIKE ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        //bind null. In SQLite LIKE NULL or NOT LIKE NULL is always false, therefore no rows expected.
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding non-string primitive value to parameter in LIKE expression is expected to work as SQLite converts the parameter value implicitly.", 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE S LIKE ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
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
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass, 5));

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 1+1+1+2";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass, 5));


    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5 OFFSET 3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass + 3, 5));

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 10/2 OFFSET 3*1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass + 3, 5));

        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 2);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (2)));
        }

        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5 - ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 3);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (2)));
        }


        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 5 OFFSET ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - 3, 5));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3)));
        }

        {
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT ? OFFSET ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - 3, 5));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (5)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3)));
        }

        {
        int pagesize = 3;
        int pageno = 2;
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT :pagesize OFFSET :pagesize * :pageno";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - (pagesize * pageno), pagesize));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pagesize", ECValue (pagesize)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pageno", ECValue (pageno)));
        }

        {
        int pagesize = 3;
        int pageno = 2;
        ecsql = "SELECT S, I FROM ecsql.PSA LIMIT :pagesize OFFSET :pagesize * (:pageno - 1)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, std::min (rowCountPerClass - (pagesize * (pageno - 1)), pagesize));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pagesize", ECValue (pagesize)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("pageno", ECValue (pageno)));
        }

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT S, I FROM ecsql.PSA LIMIT 10 OFFSET";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

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
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "FROM ONLY ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT FROM ONLY ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S WHERE L > 109222";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Select clause
    //*******************************************************
    
    ecsql = "SELECT * FROM ecsql.PSA a";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 23, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 13, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.P WHERE ECInstanceId >= 0";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 13, rowCountPerClass);

    ecsql = "SELECT a.* FROM ecsql.P a WHERE a.ECInstanceId >= 0";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 13, rowCountPerClass);

    ecsql = "SELECT * FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);


    //*******************************************************
    // Special tokens
    //*******************************************************
    //These were reserved words in the original grammar introduced by some ODBC data time functions.
    //The ODBC stuff was removed from the ECSQL grammar, and the following tests serve as safeguards
    //against regressions when updating the grammar.

    ecsql = "SELECT D FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, S FROM ecsql.PSA d";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA t";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA ts";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //same test with escaping
    ecsql = "SELECT L, B FROM ecsql.PSA Z";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //*******************************************************
    // Select clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    ecsql = "SELECT _A_B_C,_ABC,_ABC_,A_B_C_,ABC_ FROM ecsql._UnderBar";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 5, rowCountPerClass);

    ecsql = "SELECT [_A_B_C],[_ABC],[_ABC_],[A_B_C_],[ABC_] FROM ecsql.[_UnderBar]";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 5, rowCountPerClass);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::NullLiteralTests(int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "select NULL FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

    ecsql = "select NULL, I FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select NULL, NULL FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select NULL as I FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "select NULL as P3D FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "select NULL as StructProp FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "select NULL as PStruct_Array FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Alias in select clause is always interpreted literally even if it happens to be a property name.", 1, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NULL OR I IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE L IS NULL AND I IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S IS NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE S IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B IS NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS 123";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL IS NOT 123";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL <> 123";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Comparing NULL with non-NULL values are always false", 3, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE I IS ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE I IS NOT ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    {
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE ? = NULL";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    }

    {
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    }

    {
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE ? <> NULL";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    }

    {
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL <> ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    }

    {
    ecsql = "SELECT I, Dt, S FROM ecsql.PSA WHERE NULL <> ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, 0);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
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
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L ASC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L, S";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY L ASC, S DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY Dt";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY Dt ASC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY Dt DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA ORDER BY DtUtc DESC";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 ORDER BY ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA a WHERE a.ECInstanceId >= 0 ORDER BY a.ECInstanceId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY S";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY LOWER (S)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY UPPER (S)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY GetX(P3D) DESC";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 10);

    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY GetZ(P3D) ASC";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 10);

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY GetZ(P2D)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //constant value exp as order by -> no-op
    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY 1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    //boolean exp as order by
    ecsql = "SELECT I FROM ecsql.PSA WHERE I < L ORDER BY I < 123";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 10);

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY P2D DESC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY Point2D is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA tt ORDER BY tt.P2D ASC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY Point2D is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY P3D DESC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY Point3D is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY PStructProp DESC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY ECStruct is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY I_Array ASC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY arrays is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY P2D_Array DESC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY arrays is not supported by ECSQL.");

    ecsql = "SELECT I, S FROM ecsql.PSA ORDER BY PStruct_Array ASC";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "ORDER BY arrays is not supported by ECSQL.");

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
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("i", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("Sample string")));
        }

        {
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("Sample string")));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("l", ECValue (INT64_C(123456789))));
        }

        {
        //use unary operator (-) with parameter
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = -?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (-123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(-123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and don't bind anything to it
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = ?";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0); //NULL = NULL is false
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and don't bind anything to it
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?"; //NULL = -NULL is not true
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and only bind to one parameter
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE ? = -?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE :p1 > -:p1";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue("p1", ECValue(123)));
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        //and don't bind anything to it
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE :p1 > -:p1";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);
        }

        {
        //use parameters on both sides of an expression (should result in default parameter type)
        Utf8CP ecsql = "SELECT I, S FROM ecsql.PSA WHERE :p1 = -:p1";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue("p1", ECValue(123)));
        }

        {
        Utf8CP ecsql = "SELECT ?, S FROM ecsql.PSA";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
        }

        {
        Utf8CP ecsql = "SELECT ? AS NewProp, S FROM ecsql.PSA";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);
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
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT P3D FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA a";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D = P2D";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D <> P2D";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D IN (P2D, P2D)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT P3D, P2D FROM ecsql.PSA WHERE P2D NOT IN (P2D, P2D)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D >= P2D";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D <= P2D";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D BETWEEN P2D AND P2D";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D >= ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D >= ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding int to Point3D parameter is invalid.");

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (0.0, 1.0))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA a WHERE a.P2D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA a WHERE a.P2D <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        //!= NULL doesn't return any records even if LHS is not null
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D != ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        {
        ecsql = "SELECT I FROM ecsql.P WHERE P2D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding DPoint3D to Point2D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.0, 1.0, 1.0))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P2D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding int to Point2D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P2D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding string to Point2D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
        }

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.23, 22.14, 33.12))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.00, 22.00, 33.00))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P3D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding DPoint2D to Point3D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.0, 1.0))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P3D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding int to Point3D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE P3D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding string to Point3D parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
        }

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D = P3D";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D = P2D";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D IS NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P2D IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D IS NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE P3D IS NOT NULL";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (-1.3, 45.134)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (0, 0)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (0)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (-1.3)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (-1.3, 45.134, 10)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (0, 0, 0)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (0, 0)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (1, -34.1)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (P2D, P2D)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D IN (P3D, P3D)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D NOT IN (P2D, P2D)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D NOT IN (P3D, P3D)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (:p1, :p2)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR (P2D = :p2 AND P2D != :p1)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR P2D = :p2 OR P2D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint2d::From (-1.0, -2.0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (11.25, 22.16))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR (P2D != :p1 AND B = ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint2d::From (11.00, 22.00))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (true)));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P3D IN (?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.23, 22.14, 33.12))));
        }

        {
        ecsql = "SELECT I FROM ecsql.PSA WHERE P3D = :p1 OR P3D = :p2 OR P3D = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (DPoint3d::From (0.0, 0.0, 0.0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (DPoint3d::From (1.0, 1.0, 1.0))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (11.23, 22.14, 33.12))));
        }

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D IN (POINT2D (1,1), POINT2D (2,2))";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D IN (POINT3D (1,1,1), POINT3D (2,2,2))";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P2D BETWEEN POINT2D (1,1) AND POINT2D (2,2)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT I FROM ecsql.PSA WHERE P3D BETWEEN POINT3D (0,0,0) AND POINT3D (10,10,10)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "SELECT GetX(P2D), GetY(P2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT GetX(P3D), GetY(P3D), GetZ(P3D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT GetZ(P2D) FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::PolymorphicTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, L FROM ONLY ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    //Do non-polymorphic query on TablePerHierarchy    
    ecsql = "SELECT * FROM ONLY ecsql.THBase";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH2";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH4";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TH5";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass);

    //Do polymorphic query on TablePerHierarchy    
    ecsql = "SELECT * FROM ecsql.THBase";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass * 6);

    ecsql = "SELECT * FROM ecsql.TH1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass * 5);

    ecsql = "SELECT * FROM ecsql.TH2";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass * 4);

    ecsql = "SELECT * FROM ecsql.TH3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass * 3);

    ecsql = "SELECT * FROM ecsql.TH4";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass * 2);

    ecsql = "SELECT * FROM ecsql.TH5";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass * 1);

    //Do non-polymorphic query on TablePerClass    
    ecsql = "SELECT * FROM ONLY ecsql.TCBase";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC2";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC4";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass);

    ecsql = "SELECT * FROM ONLY ecsql.TC5";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass);

    //Do polymorphic query on TablePerClass    
    ecsql = "SELECT * FROM ecsql.TCBase";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1 + 1, rowCountPerClass * 6);

    ecsql = "SELECT * FROM ecsql.TC1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2 + 1, rowCountPerClass * 5);

    ecsql = "SELECT * FROM ecsql.TC2";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3 + 1, rowCountPerClass * 4);

    ecsql = "SELECT * FROM ecsql.TC3";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 4 + 1, rowCountPerClass * 3);

    ecsql = "SELECT * FROM ecsql.TC4";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5 + 1, rowCountPerClass * 2);

    ecsql = "SELECT * FROM ecsql.TC5";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 6 + 1, rowCountPerClass * 1);

    ecsql = "select ECInstanceId FROM ecsql.Empty";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "select ECInstanceId, I, S from ecsql.AbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "select ECInstanceId from ecsql.EmptyAbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "select ECInstanceId, I, S from only ecsql.AbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "select ECInstanceId from only ecsql.EmptyAbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::PrimitiveTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT I, L FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, L FROM ecsql.PSA a";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT a.I, a.L FROM ecsql.PSA a";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT B, ECInstanceId, S FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT Bi FROM ecsql.PSA a";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT 3.14 FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT 3.14 FROM ecsql.PSA WHERE L = 0";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 0);

    ecsql = "SELECT 1000 AS I FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT 3.14 AS BlaBla FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (12)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = :p";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p", ECValue (123)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE I = :p";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p", ECValue (12)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = :p1 OR I = :p2";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (INT64_C(12))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (123)));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = :p1 OR I = :p2";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (INT64_C(12))));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = ? OR I = :p1 OR B = :p2 OR S = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (INT64_C(100))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p2", ECValue (true)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("blabla")));
        }

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE L = ? OR I = :p1 OR I > :p1 OR S = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (INT64_C(100))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("p1", ECValue (120)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("blabla")));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding string to integer parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("blabla")));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding double to integer parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3.14)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding boolean to integer parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (true)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE I <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding DPointXD to int parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.0, 1.0))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE D <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Binding DateTime to double parameter is invalid.");
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 9, 12)));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE D > ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding long to double parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (INT64_C(1))));
        }

    //***** Boolean properties
    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = true";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = True";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = false";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = False";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = Unknown";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE B = UNKNOWN";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        {
        ecsql = "SELECT I FROM ecsql.P WHERE B = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding long to boolean parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (INT64_C(1))));
        }

        {
        ecsql = "SELECT I FROM ecsql.P WHERE B <> ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, ECSqlExpectedResult::Category::Supported, "Binding long to boolean parameter is no error as SQLite supports that.", 1, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("bla bla")));
        }

    //***** String literals
    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S = 'Sample string'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S = 'Sample \"string'";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S = \"Sample string\"";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "String literals must be surrounded by single quotes. Double quotes are equivalent to square brackets in SQL.");

     ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S_Array = 123";
     ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DataType mismatch.");

     ecsql = "SELECT I, S, B FROM ecsql.PSA WHERE S_Array = '123'";
     ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Type mismatch in array.");

     ecsql = "SELECT * FROM ecsql.PSA WHERE B = Invalid";
     ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Not a valid option.");
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::SelectClauseTests(int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //tests with identically named select clause items. If one of them is an alias, preparation fails. Otherwise a unique name is generated

    Utf8CP ecsql = "select NULL, NULL bla FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select NULL, NULL FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select NULL, NULL, NULL FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 3, rowCountPerClass);

    ecsql = "select NULL bli, NULL bla FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select NULL bla, NULL bla FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select NULL I, I FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select I, I FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select I, L AS I FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select L AS I, I FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select I + L, I + L FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select I + L, I +L FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

    ecsql = "select L, GetECClassId() L FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "select GetECClassId() S, S FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::SourceTargetConstraintTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT SourceECClassId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT GetECClassId (), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.PSAHasP_N1";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 5, 0);

    ecsql = "SELECT TargetECClassId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //link table mapping
    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasPSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    //end table mapping
    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP WHERE SourceECClassId = TargetECClassId AND GetECClassId () = 180";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT rel.GetECClassId (), rel.SourceECClassId, rel.TargetECClassId FROM ecsql.PSAHasP rel WHERE rel.SourceECClassId = rel.TargetECClassId AND rel.GetECClassId () = 180";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP ORDER BY GetECClassId (), SourceECClassId, TargetECClassId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT GetECClassId (), SourceECClassId, TargetECClassId FROM ecsql.PSAHasP rel ORDER BY rel.GetECClassId (), rel.SourceECClassId, rel.TargetECClassId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    ecsql = "SELECT rel.GetECClassId (), rel.SourceECClassId, rel.TargetECClassId FROM ecsql.PSAHasP rel";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 0);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::StructTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT PStructProp.i, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    //Struct member property I does not exist
    ecsql = "SELECT PStructProp.I, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT PStructProp FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT B, PStructProp FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT B, PStructProp, P3D FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, rowCountPerClass);

    ecsql = "SELECT SAStructProp FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT SAStructProp.PStructProp FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT SAStructProp.PStructProp, SAStructProp.PStructProp.p3d FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

    ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp = ?";
    ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);

        {
        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i = ?";
        auto& testItem = ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        ecsql = "SELECT I, S FROM ecsql.PSA tt WHERE tt.PStructProp = ?";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp IS NULL";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, 0);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp IS NOT NULL";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NULL";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 0);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NOT NULL";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp IS NULL";
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Structs that contain struct arrays are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp IS NOT NULL";
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Structs that contain struct arrays are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp.PStructProp IS NULL";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, 0);

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp.PStructProp IS NOT NULL";
        ECSqlTestFrameworkHelper::AddSelect(dataset, ecsql, 1, rowCountPerClass);

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE PStruct_Array IS NULL";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Struct arrays are not supported in where clause.");

        ecsql = "SELECT ECInstanceId FROM ecsql.SA WHERE PStruct_Array IS NOT NULL";
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Struct arrays are not supported in where clause.");

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NOT NULL";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IN (10, 123, 200)";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);

        ecsql = "SELECT I, S FROM ecsql.PSA WHERE PStructProp.i BETWEEN 10 AND 200";
        ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, rowCountPerClass);
        
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::SubqueryTests( int rowCountPerClass )
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT ECInstanceId FROM ecsql.P WHERE ECInstanceId < (SELECT avg(ECInstanceId) FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass/2);

    ecsql = "SELECT ECInstanceId FROM (SELECT * FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass);

    ecsql = "SELECT ECInstanceId FROM (SELECT COUNT(*) FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT ECInstanceId FROM (SELECT * FROM P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Schema prefix not mentioned before class name.");

    ecsql = "SELECT ECInstanceId FROM (SELECT * FROM sql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Invalid Schema prefix.");

    ecsql = "SELECT * FROM (SELECT A FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Property 'A' does not match with any of the class properties.");

    ecsql = "SELECT * FROM ecsql.P WHERE (SELECT * FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "SubQuery should return exactly 1 coloumn.");

    ecsql = "SELECT AVG(S) FROM (SELECT * FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "AVG function can only be applied to numeric values.");

    ecsql = "SELECT * FROM (SELECT I FROM ecsql.P HAVING COUNT(S)>1)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "A GROUP BY clause is mandatory before HAVING.");

    ecsql = "SELECT * FROM ecsql.PSA WHERE (SELECT ? FROM ecsql.PSA WHERE I=abc)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Propert 'I' doesn't take String values.");

    ecsql = "SELECT L FROM (SELECT * FROM ecsql.P WHERE B <>)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Property B is not assigned any value.");

    ecsql = "SELECT L FROM ecsql.PSA WHERE(SELECT * FROM ecsql.P WHERE I BETWEEN 100)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Syntax error in query.Expecting AND.");

    ecsql = "SELECT * FROM ecsql.P WHERE B = (SELECT * FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Outer clause expecting a single value whereas inner returns multiple.");

    ecsql = "SELECT * FROM ecsql.PSA WHERE B = (SELECT DateOnly FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Left and right of the expression should have the same data type.");

    ecsql = "SELECT * FROM ecsql.PSA WHERE B IN (SELECT DateOnly FROM ecsql.P)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Left and right of the expression should have the same data type.");
    
    ecsql = "SELECT * FROM (SELECT I FROM ecsql.P WHERE COUNT(S)>1)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "WHERE clause can't be used with aggregate functions.");

    ecsql = "SELECT COUNT(?) FROM ecsql.PSA WHERE (SELECT I FROM ecsql.PSA WHERE I=?)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Parameter binding required.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlSelectTestDataset::UnionTests(int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "SELECT ECInstanceId FROM ecsql.P UNION SELECT ECInstanceId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass*2);;

    ecsql = "SELECT ECInstanceId FROM ecsql.P UNION ALL SELECT ECInstanceId FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, rowCountPerClass*2);

    ecsql = "SELECT B, Bi, I, L, S, P2D, P3D FROM ecsql.P UNION ALL SELECT B, Bi, I, L, S, P2D, P3D FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 7, rowCountPerClass*2);

    ecsql = "SELECT PStructProp FROM ecsql.PSA UNION SELECT SAStructProp.PStructProp FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, 1);

    ecsql = "SELECT B_Array, Bi_Array, D_Array, Dt_Array, I_Array, S_Array, P2D_Array, P3D_Array FROM ecsql.PSA UNION ALL SELECT B_Array, Bi_Array, D_Array, Dt_Array, I_Array, S_Array, P2D_Array, P3D_Array FROM ecsql.PA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 8, rowCountPerClass*2);

    ecsql = "SELECT PStruct_Array FROM ecsql.PSA UNION SELECT SAStructProp.PStruct_Array FROM ecsql.SA";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1,  20);

    ecsql = "SELECT ECClassId, COUNT(*) FROM (SELECT GetECClassId() ECClassId, ECInstanceId FROM ecsql.PSA UNION ALL SELECT GetECClassId() ECClassId, ECInstanceId FROM ecsql.SA) GROUP BY ECClassId";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 2, 2);

    ecsql = "SELECT ECClassId, Name, COUNT(*) FROM (SELECT GetECClassId() ECClassId, ECInstanceId, 'PSA' Name FROM ecsql.PSA UNION ALL SELECT GetECClassId() ECClassId, ECInstanceId, 'SA' Name FROM ecsql.SA) GROUP BY ECClassId, Name";
    ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 3, 2);


    ecsql = "SELECT S FROM ecsql.P UNION SELECT Dt FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT Bi FROM ecsql.P UNION SELECT I_Array FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT Bi_Array FROM ecsql.PSA UNION SELECT I_Array FROM ecsql.PA";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT * FROM ecsql.P UNION SELECT * FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT S FROM ecsql.P UNION SELECT * FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Number and Type of properties should be same in all the select clauses.");

    ecsql = "SELECT B FROM ecsql.P UNION SELECT B_Array FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT * FROM ecsql.P UNION ALL SELECT * FROM ecsql.PA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "SELECT * FROM ecsql.P UNION ALL SELECT * FROM ecsql.A";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "'A' is not a valid table name.");
    return dataset;
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE