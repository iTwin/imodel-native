/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlUpdateTestDataset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlUpdateTestDataset.h"

//Note: Please keep methods for a given class alphabetized

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::ArrayTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Dt_Array = NULL, B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET PStruct_Array = NULL, B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::CommonGeometryTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.PASpatial SET I = 123, Geometry = NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PASpatial SET I = 123, Geometry_Array = NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PASpatial SET I = 123 WHERE Geometry IS NOT NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PASpatial SET I = 123 WHERE Geometry_Array IS NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::DateTimeTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //updating date time prop without DateTimeInfo CA

    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55.123'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55.123456'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2013-02-18T06:00:00.000'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55Z'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = DATE '2012-01-18'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE Dt = TIMESTAMP '2012-01-18 13:02:55Z'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //Updating UTC date time prop
    ecsql = "UPDATE ONLY ecsql.P SET DtUtc = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = TIMESTAMP '2012-01-18 13:02:55Z'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET DtUtc = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while value is not.");

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = TIMESTAMP '2012-01-18 13:02:55'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while value is not.");

    ecsql = "UPDATE ONLY ecsql.P SET DtUtc = DATE '2012-01-18'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //Updating date time prop with DateTimeInfo CA where kind is set to Unspecified
    ecsql = "UPDATE ONLY ecsql.P SET DtUnspec = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUnspec = TIMESTAMP '2012-01-18 13:02:55'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET DtUnspec = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.");

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUnspec = TIMESTAMP '2012-01-18 13:02:55Z'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.");

    ecsql = "UPDATE ONLY ecsql.P SET DtUnspec = DATE '2012-01-18'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //Updating date time props with DateTimeInfo CA where component is set to Date-onlys
    ecsql = "UPDATE ONLY ecsql.P SET DateOnly = DATE '2013-02-18'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //DateOnly can take time stamps, too
    ecsql = "UPDATE ONLY ecsql.P SET DateOnly = TIMESTAMP '2013-02-18 06:00:00Z'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DateOnly = TIMESTAMP '2012-01-18 13:02:55'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET DateOnly = TIMESTAMP '2013-02-18 06:00:00'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //CURRENT_XXX functions
    ecsql = "UPDATE ONLY ecsql.P SET Dt = CURRENT_DATE";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET DtUtc = CURRENT_DATE";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = CURRENT_DATE";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET Dt = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET DtUtc = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.P SET DtUnspec = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp");

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE DtUnspec = CURRENT_TIMESTAMP";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp");

    ecsql = "UPDATE ONLY ecsql.P SET Dt = CURRENT_TIME";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    ecsql = "UPDATE ONLY ecsql.P SET I = 123 WHERE Dt = CURRENT_TIME";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    //*** Parameters ****
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I=123, Dt=?, DtUtc=?, DtUnspec=?, DateOnly=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I=123, Dt=?, DtUtc=? WHERE DtUnspec=? AND DateOnly=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET Dt=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET Dt=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET Dt=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET Dt=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUtc=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUtc=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUtc=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUtc=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }


    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUnspec=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUnspec=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUnspec=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DtUnspec=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }


    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DateOnly=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DateOnly=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DateOnly=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET DateOnly=?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::FunctionTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = 123, L = GetECClassId ()";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::MiscTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    // Syntactically incorrect statements 
    //*******************************************************
    Utf8CP ecsql = "";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSA WHERE I = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Typical updates
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql.PSA SET I = 124, L = 100000000000, D = -1.2345678, S = 'hello, world'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    
    ecsql = "UPDATE ONLY ecsql.PSA SET Dt = ?, L = ?";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Class aliases
    //*******************************************************
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them
    //during preparation
    ecsql = "UPDATE ONLY ecsql.PSA t SET t.I = 124, t.L = 100000000000, t.D = -1.2345678, t.S = 'hello, world' WHERE t.D > 0.0";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA t SET t.Dt = ?, t.L = ?";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Update ECInstanceId 
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql.PSA SET ECInstanceId = -3, I = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSA SET [ECInstanceId] = -3, I = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported, "The bracketed property [ECInstanceId] refers to an ECProperty (and not to the system property ECInstanceId). Parsing [ECInstanceId] is not yet supported.");

    //*******************************************************
    //  Literals
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql.PSA SET B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET B = false";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET B = UNKNOWN";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "UPDATE ONLY ecsql.PSA SET Dt = DATE '2012-01-18'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET Dt = TIMESTAMP '2012-01-18T13:02:55'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET Dt = TIME '13:35:16'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "UPDATE ONLY ecsql.PSA SET Dt = LOCALTIME";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

    ecsql = "UPDATE ONLY ecsql.PSA SET P2D = POINT2D (-1.3, 45.134)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "UPDATE ONLY ecsql.PSA SET P3D = POINT3D (-1.3, 45.134, 2)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    //*******************************************************
    // Update clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql._UnderBar u SET u._A_B_C = '1st Property', u._ABC = 22, u._ABC_ = '3rd Property', u.A_B_C_ = 44, u.ABC_= 'Last Property' WHERE u._ABC > 0";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.[_UnderBar] u SET u.[_A_B_C] = '1st Property', u.[_ABC] = 22, u.[_ABC_] = '3rd Property', u.[A_B_C_] = 44, u.[ABC_]= 'Last Property' WHERE u.[_ABC] > 0";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    //*******************************************************
    // update clause where string literal consists of Escaping single quotes
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql._UnderBar u SET u._A_B_C = '''', u._ABC = 22, u._ABC_ = '''5''', u.A_B_C_ = 44, u.ABC_= 'LAST''' WHERE u._ABC > 0";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::ParameterAdvancedTests (int rowCountPerClass)
    {
    //This includes only advanced parameter tests that are not covered implicitly by the other test datasets

    ECSqlTestDataset dataset;

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = 123, S = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = ?, S = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
    }

    //Blob
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = 123, Bi = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    Byte blob[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00 };
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (blob, 10)));
    }

    //Points
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = 123, P2D = ?, P3D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
    }

    //binding null
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = ?, S = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
    //bind null. 
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.P SET I = ?, S = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    //bind null. 
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
    }

    //reusing named parameter
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET I = :i, S = :s, L = (:i - 23) * 100";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("i", ECValue (123)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("hello")));
    }

    //mixing unnamed and named parameters
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET I = ?, S = :s, L = :l";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("hello")));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("l", ECValue (INT64_C(123456789))));
    }

    //type match tests
    {
    //primitive types except for points and date times are convertible into each other (by SQLite)
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Bi = ?, I = ?, S = ?, L = ?, B = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (true)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
    Byte blob[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00 };
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (blob, 10)));
    }

    //Date time <-> basic primitive types
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Dt = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET I = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Dt = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1.1)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    //Date time <-> Point2D
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Dt = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P2D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    //Date time <-> Point3D
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Dt = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P3D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    //Date time <-> Structs
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    //Date time <-> Arrays
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET Dt_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStruct_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    //Point2D <-> basic primitive types
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P2D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1.1)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
    }

    //Point2D <-> P3D
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P2D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P3D = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
    }

    //Point2D <-> Structs
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
    }

    //Point2D <-> Arrays
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P2D_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStruct_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
    }

    //Point3D <-> Structs
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
    }

    //Point3D <-> Arrays
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET P3D_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStruct_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
    }

    //structs <-> primitives
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
    }

    //arrays <-> primitives
    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET I_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
    }

    {
    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStruct_Array = ?";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::PolymorphicTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ecsql.PSA SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ecsql.Abstract SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.Abstract SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ecsql.AbstractNoSubclasses SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.AbstractNoSubclasses SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ecsql.AbstractTablePerHierarchy SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.AbstractTablePerHierarchy SET I = 123";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ecsql.THBase SET S = 'hello'";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.THBase SET S = 'hello'";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "UPDATE ecsql.TCBase SET S = 'hello'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.TCBase SET S = 'hello'";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    return dataset;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::RelationshipEndTableMappingTests(int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.PSAHasP SET SourceECInstanceId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET SourceECInstanceId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET TargetECInstanceId = 134";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET TargetECInstanceId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET SourceECClassId = 111";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET SourceECClassId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET TargetECClassId = 111";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasP SET TargetECClassId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::RelationshipLinkTableMappingTests(int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET SourceECInstanceId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET SourceECInstanceId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET TargetECInstanceId = 134";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET TargetECInstanceId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET SourceECClassId = 111";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET SourceECClassId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET TargetECClassId = 111";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPSA SET TargetECClassId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::RelationshipWithAdditionalPropsTests (ECDbR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

        {
        Savepoint savepoint (ecdb, "Inserting test instances");
        const auto ecInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (100, 200)");
        if (!ecInstanceId.IsValid ())
            {
            savepoint.Cancel ();
            return dataset;
            }

        savepoint.Commit ();
        }


    Utf8CP ecsql = "UPDATE ONLY ecsql.PSAHasPWithPrimProps SET SourceECInstanceId = 400, TargetECInstanceId = 234;";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasPWithPrimProps SET D = 3.14, B = false";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSAHasPWithPrimProps SET D = 3.14, B = false WHERE D IS NULL AND B IS NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::RelationshipWithAnyClassConstraintTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.PSAHasAnyClass_0N SET SourceECInstanceId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasAnyClass_0N SET TargetECInstanceId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasAnyClass_0N SET SourceECClassId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSAHasAnyClass_0N SET TargetECClassId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.AnyClassHasP_0N SET SourceECInstanceId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.AnyClassHasP_0N SET TargetECInstanceId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.AnyClassHasP_0N SET SourceECClassId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.AnyClassHasP_0N SET TargetECClassId = 123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::StructTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp = NULL, B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp = ?, B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp.i = 123, B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.PSA SET PStructProp.i = 123, PStructProp.dt = DATE '2010-10-10', B = true";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp = NULL";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp = ?";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp.i = 123, SAStructProp.PStructProp.dt = DATE '2010-10-10'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlUpdateTestDataset::TargetClassTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    //Updating classes with base classes
    //*******************************************************
    Utf8CP ecsql = "UPDATE ONLY ecsql.TH5 SET S='hello', S1='hello1', S3='hello3', S5='hello5'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Updating structs
    //*******************************************************
    //structs which are domain classes
    ecsql = "UPDATE ONLY ecsql.SAStruct SET PStructProp.i=123, PStructProp.l=100000, PStructProp.dt=DATE '2013-10-10', PStructProp.b=False";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //structs which are not domain classes. They cannot be updated, so this always returns 0 rows affected.
    ecsql = "UPDATE ONLY ecsql.PStruct SET i=123, l=10000, dt=DATE '2013-10-10', b=False";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Updating CAs
    //*******************************************************
    ecsql = "UPDATE ONLY bsca.DateTimeInfo SET DateTimeKind='Utc'";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Custom Attributes classes are invalid in UPDATE statements.");


    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql.PUnmapped SET I=123, D=3.14";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in UPDATE statements.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    //by contract non-polymorphic updates on abstract classes are valid, but are a no-op
    ecsql = "UPDATE ONLY ecsql.Abstract SET I=123, S='hello'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "UPDATE ONLY ecsql.AbstractNoSubclasses SET I=123, S='hello'";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Subclasses of abstract class
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql.Sub1 SET I=123, S='hello', Sub1I=100123";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Empty classes
    //*******************************************************
    ecsql = "UPDATE ONLY ecsql.Empty SET ECInstanceId = ?";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Empty classes cannot be used in UPDATE statements.");


    //*******************************************************
    // Unsupported classes
    //*******************************************************
    //AnyClass is unsupported, but doesn't have properties, so it cannot be used in an UPDATE statement because of that in the first place

    ecsql = "UPDATE ONLY bsm.InstanceCount SET ECSchemaName='Foo', ECClassName='Goo', Count=103";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "UPDATE ONLY PSA SET I=123, L=100000";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema prefix.");

    ecsql = "UPDATE ONLY ecsql.BlaBla SET I=123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY blabla.PSA SET I=123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "UPDATE ONLY ecsql.PSA SET Garbage='bla', I=123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "One of the properties does not exist in the target class.");

    return dataset;
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE