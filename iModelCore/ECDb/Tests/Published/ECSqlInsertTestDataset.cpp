/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlInsertTestDataset.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlInsertTestDataset.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
const std::vector<Utf8String> ECSqlInsertTestDataset::s_invalidClassIdValues = {Utf8String("'bla'"), Utf8String("3.14"), Utf8String("True"), Utf8String("DATE '2013-10-10'")};

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::ArrayTests()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt_Array, B) VALUES (NULL, true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStruct_Array, B) VALUES (NULL, true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::CommonGeometryTests()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PASpatial VALUES (False, 3.14, 123, 'hello', NULL, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    {
    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(line));
    }

    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, ?)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::DateTimeTests()
    {
    ECSqlTestDataset dataset;

    //Inserting into date time prop without DateTimeInfo CA

    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123456')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2013-02-18T06:00:00.000')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55Z')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //Inserting into UTC date time prop
    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while value is not.");

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //Inserting into date time prop with DateTimeInfo CA where kind is set to Unspecified
    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.");

    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //Inserting into date time props with DateTimeInfo CA where component is set to Date-onlys
    ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (DATE '2013-02-18')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //DateOnly can take time stamps, too
    ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //CURRENT_XXX functions
    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (CURRENT_DATE)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_DATE)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIMESTAMP)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_TIMESTAMP)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (CURRENT_TIMESTAMP)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp");

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIME)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    //*** Parameters ****
    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, Dt, DtUtc, DtUnspec, DateOnly) VALUES (123, ?, ?, ?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 2, 18)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 2, 18)));
    }



    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 2, 18)));
    }


    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 2, 18)));
    }


    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 2, 18)));
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::FunctionTests()
    {
    ECSqlTestDataset dataset;
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::IntoTests()
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    // Inserting into classes which map to tables with ECClassId columns
    //*******************************************************
    Utf8CP ecsql = "INSERT INTO ecsql.THBase (S) VALUES ('hello')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //inserting into classes with base classes
    ecsql = "INSERT INTO ecsql.TH5 (S, S1, S3, S5) VALUES ('hello', 'hello1', 'hello3', 'hello5')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.TH3 VALUES ('hello', 'hello1', 'hello2', 'hello3')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //*******************************************************
    // Inserting into structs
    //*******************************************************
    //structs are not insertible
    ecsql = "INSERT INTO ecsql.PStruct (i, l, dt, b) VALUES (123, 1000000, DATE '2013-10-10', False)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.PStruct VALUES (False, NULL, 3.14, DATE '2013-10-10', TIMESTAMP '2013-10-10T12:00Z', 123, 10000000, 'hello', NULL, NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Inserting into CAs
    //*******************************************************
    ecsql = "INSERT INTO bsca.DateTimeInfo (DateTimeKind) VALUES ('Utc')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Custom Attributes classes are invalid in INSERT statements.");


    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "INSERT INTO ecsql.PUnmapped (I, D) VALUES (123, 3.14)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in INSERT statements.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    ecsql = "INSERT INTO ecsql.Abstract (I, S) VALUES (123, 'hello')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Abstract classes cannot be used in INSERT statements.");

    ecsql = "INSERT INTO ecsql.AbstractNoSubclasses (I, S) VALUES (123, 'hello')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Abstract classes cannot be used in INSERT statements.");

    //*******************************************************
    // Subclasses of abstract class
    //*******************************************************
    ecsql = "INSERT INTO ecsql.Sub1 (I, S, Sub1I) VALUES (123, 'hello', 100123)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //*******************************************************
    // Empty classes
    //*******************************************************
    ecsql = "INSERT INTO ecsql.Empty (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //*******************************************************
    // Unsupported classes
    //*******************************************************
    //AnyClass is unsupported, but doesn't have properties, so it cannot be used in an INSERT statement because of that in the first place

    ecsql = "INSERT INTO bsm.InstanceCount (ECSchemaName, ECClassName, Count) VALUES ('Foo', 'Goo', 103)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema alias / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "INSERT INTO PSA (I, L, Dt) VALUES (123, 1000000, DATE '2013-10-10')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema alias.");

    ecsql = "INSERT INTO ecsql.BlaBla VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO blabla.PSA VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.PSA (Garbage, I, L) VALUES ('bla', 123, 100000000)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "One of the properties does not exist in the target class.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::MiscTests(ECDbR ecdb)
    {
    ECSqlTestDataset dataset;

    ECInstanceId psaInstanceId;
    ECInstanceId pInstanceId;
    {
    Savepoint savepoint(ecdb, "Inserting test instances", true);
    psaInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");

    if (!psaInstanceId.IsValid() || pInstanceId.IsValid())
        {
        savepoint.Cancel();
        return dataset;
        }

    savepoint.Commit();
    }

    //*******************************************************
    // Syntactically incorrect statements 
    //*******************************************************
    Utf8CP ecsql = "";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT ecsql.P (I) VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (I)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Insert expressions 
    //*******************************************************
    ecsql = "INSERT INTO ecsql.P (I) VALUES (1 + 1)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (I) VALUES (5 * 4)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (L) VALUES (1 + ECClassId)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (L) VALUES (ECClassId * 4)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    //*******************************************************
    // Insert ECInstanceId 
    //*******************************************************
    //NULL for ECInstanceId means ECDb auto-generates the ECInstanceId.
    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (NULL)"; //table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (ECInstanceId, I) VALUES (NULL, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId, S2) VALUES (NULL, 'hello')"; //table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (123)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (4443412341)"; //table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    //If ECInstanceId is specified via parameter, parameter must be bound.
    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (?)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    {
    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECInstanceId(UINT64_C(141231498))));
    }

    {
    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (?)";//table per hierarchy mapping->class id must be populated
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECInstanceId(UINT64_C(141231498))));
    }

    ecsql = "INSERT INTO ecsql.PSAHasP (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.PSAHasPSA (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (ECInstanceId, I) VALUES (123, 123)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId, S1) VALUES (41241231231, 's1')";//table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    //for link table mappings specifying the ECInstanceId is same as for regular classes
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (NULL, %s, %s);", psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (129, %s, %s);", psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (?, %s, %s);", psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECInstanceId(UINT64_C(145))));
    }

    //for end table mappings specifying the ECInstanceId is a no-op. It will be ignored.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (NULL, %s, ?);", psaInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(pInstanceId));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (?, %s, ?);", psaInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(pInstanceId));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(pInstanceId));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (14123123, %s, ?);", psaInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(pInstanceId));
    }

    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (123)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, false);
    //provoke pk constraint violation as instance with id 123 already exists
    ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (412313)";//table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, false);
    //provoke pk constraint violation as instance with id 412313 already exists
    ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Class aliases
    //*******************************************************
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them
    //during preparation
    ecsql = "INSERT INTO ecsql.PSA t (t.Dt, t.L, t.S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Insert literal values
    //*******************************************************
    ecsql = "INSERT INTO ecsql.PSA (Dt, L, S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (L, S, I) VALUES (100000000000, 'hello, \" world', -1)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (L, I) VALUES (CAST (100000 AS INT64), 12 + 99)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (L, S, DtUtc) VALUES (?, ?, ?)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //*******************************************************
    // Insert without column clause
    //*******************************************************
    ecsql = "INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //*******************************************************
    //  VALUES clause mismatch
    //*******************************************************
    ecsql = "INSERT INTO ecsql.P (I, L) VALUES ('bla', 123, 100000000)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (I, L) VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (I, L) VALUES (123, DATE '2013-04-30')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P VALUES (123, 'bla bla')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    //  Literals
    //*******************************************************
    ecsql = "INSERT INTO ecsql.PSA (B) VALUES (true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (B) VALUES (True)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (B) VALUES (false)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (B) VALUES (UNKNOWN)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (TIMESTAMP '2012-01-18T13:02:55')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (TIME '13:35:16')";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (LOCALTIME)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

    ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (POINT2D (-1.3, 45.134))";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "INSERT INTO ecsql.PSA (P3D) VALUES (POINT3D (-1.3, 45.134, 2))";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    //*******************************************************
    // Not yet supported flavors
    //*******************************************************
    ecsql = "INSERT INTO ecsql.P (I, L) SELECT I, L FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "INSERT INTO ecsql.P (I, L) VALUES (1, 1234), (2, 32434)";
    ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    //*******************************************************
    // Insert clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    ecsql = "INSERT INTO ecsql._UnderBar (_A_B_C, _ABC, _ABC_, A_B_C_, ABC_) VALUES ('_A_B_C', 2, '_ABC_', 4, 'ABC_')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //insert clause with square brackets
    ecsql = "INSERT INTO ecsql.[_UnderBar] ([_A_B_C], [_ABC], [_ABC_], [A_B_C_], [ABC_]) VALUES ('_A_B_C', 2, '_ABC_', 4, 'ABC_')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    //*******************************************************
    // Insert query where string literal consists of Escaping single quotes
    //*******************************************************
    ecsql = "INSERT INTO ecsql._UnderBar (_A_B_C, _ABC, _ABC_, A_B_C_, ABC_) VALUES ('5''55''', 2, '''_ABC_', 4, 'ABC_''')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::ParameterAdvancedTests()
    {
    //This includes only advanced parameter tests that are not covered implicitly by the other test datasets

    ECSqlTestDataset dataset;

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (123, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue("hello")));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue("hello")));
    }

    //Blob
    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, Bi) VALUES (123, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    Byte blob[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00};
    ECValue binaryvalue;
    binaryvalue.SetBinary(blob, 10, true);  // NB: Pass true for holdADuplicate, as the default is false
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(binaryvalue));
    }

    //Points
    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, P2D, P3D) VALUES (123, ?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint2d::From(1.1, 2.2))));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint3d::From(1.1, 2.2, 3.3))));
    }

    //binding null
    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
    //bind null. 
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    //bind null. 
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    }

    //reusing named parameter
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (I, S, L) VALUES (:i, :s, (:i - 23) * 100)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("i", ECValue(123)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("s", ECValue("hello")));
    }

    //mixing unnamed and named parameters
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (I, S, L) VALUES (?, :s, :l)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("s", ECValue("hello")));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("l", ECValue(INT64_C(123456789))));
    }

    //type match tests
    {
    //primitive types except for points and date times are convertible into each other (by SQLite)
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Bi, I, S, L, B) VALUES (?, ?, ?, ?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue("hello")));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(true)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue("hello")));
    Byte blob[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00};
    ECValue binaryvalue;
    binaryvalue.SetBinary(blob, 10, true);  // NB: Pass true for holdADuplicate, as the default is false
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(binaryvalue));
    }

    //Date time <-> basic primitive types
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(1)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (I) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(1.1)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    //Date time <-> Point2D
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint2d::From(1.1, 2.2))));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    //Date time <-> Point3D
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint3d::From(1.1, 2.2, 3.3))));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P3D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    //Date time <-> Structs
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    //Date time <-> Arrays
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    //Point2D <-> basic primitive types
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(1.1)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(DateTime(2013, 1, 1)));
    }

    //Point2D <-> P3D
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint3d::From(1.1, 2.2, 3.3))));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P3D) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint2d::From(1.1, 2.2))));
    }

    //Point2D <-> Structs
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint2d::From(1.1, 2.2))));
    }

    //Point2D <-> Arrays
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint2d::From(1.1, 2.2))));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint2d::From(1.1, 2.2))));
    }

    //Point3D <-> Structs
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint3d::From(1.1, 2.2, 3.3))));
    }

    //Point3D <-> Arrays
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (P3D_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint3d::From(1.1, 2.2, 3.3))));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(DPoint3d::From(1.1, 2.2, 3.3))));
    }

    //structs <-> primitives
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
    }

    //arrays <-> primitives
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (I_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(123)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(1)));
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipEndTableMappingTests(ECDbR ecdb)
    {
    ECClassId psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    ECClassId pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");
    ECClassId thBaseClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "THBase");
    ECClassId th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");

    ECSqlTestDataset dataset;

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId p2ECInstanceId;
    ECInstanceId thBaseECInstanceId;
    ECInstanceId th3ECInstanceId;

    {
    Savepoint savepoint(ecdb, "Inserting test instances", true);
    psaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
    p2ECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.P (I, S) VALUES (200, 'Test instance for relationship tests')");
    thBaseECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.THBase (S) VALUES ('Test instance for relationship tests')");
    th3ECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.TH3 (S3) VALUES ('Test instance for relationship tests')");

    Utf8String testRelationshipECSql;
    testRelationshipECSql.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s)",
                                  psaECInstanceId.ToString().c_str(), p2ECInstanceId.ToString().c_str());

    ECInstanceId relECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, testRelationshipECSql.c_str());

    if (!psaECInstanceId.IsValid() || !pECInstanceId.IsValid() || !p2ECInstanceId.IsValid() ||
        !thBaseECInstanceId.IsValid() || !th3ECInstanceId.IsValid() || !relECInstanceId.IsValid())
        {
        savepoint.Cancel();
        return dataset;
        }

    savepoint.Commit();
    }

    //**** Case: source/target class id not mandatory as source/target constraints are unambiguous
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     thBaseECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Invalid,"FK violation as source id doesn't match", true);
    }

    {
    //this end is incorrect -> expects failure
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Invalid);
    }

    {
    //This end already has a relationship of this type -> expects failure
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaECInstanceId.ToString().c_str(), p2ECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), pECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    //class id in this case is optional, so it is not an error to not bind to it
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str(), pClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(pClassId.GetValue())));
    }

    {
    //class id in this case is optional, so it is not an error to not bind to it
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), pECInstanceId.ToString().c_str(), pClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //Target is non-polymorphic, so only THBase items are allowed -> class id should not be necessary
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //specifying mismatching class ids -> no validation done by ECDb
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), th3ECInstanceId.ToString().c_str(), th3ClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //specifying mismatching class ids -> no validation done by ECDb
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str(), psaClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //specifying mismatching class ids -> no validation done by ECDb
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), pClassId.ToString().c_str(), pECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //specifying mismatching class ids -> no validation done by ECDb
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP VALUES (%s, %s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), pClassId.ToString().c_str(), pECInstanceId.ToString().c_str(), psaClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //**** Case:  target class id mandatory as target constraint is ambiguous

    //Now target is polymorphic, which means that THBase and all subclasses are valid -> target class id is necessary (but source class id not)
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), thBaseECInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    {
    //class id is never validated, so can also be null
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //specifying mismatching class ids is not validated
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str(), pClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //specifying mismatching class ids is not validated
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(pClassId.GetValue())));
    }

    //specifying non-sense class ids. They are not validated and left up to SQLite to handle them - which usually converts them to an integer
    for (Utf8StringCR nonSenseClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                         psaECInstanceId.ToString().c_str(), nonSenseClassIdValue.c_str(), thBaseECInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
        ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);

        ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                         psaECInstanceId.ToString().c_str(), pECInstanceId.ToString().c_str(), nonSenseClassIdValue.c_str());
        ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipLinkTableMappingTests(ECDbR ecdb)
    {
    ECClassId psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    ECClassId thBaseClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "THBase");
    ECClassId th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");

    ECSqlTestDataset dataset;

    ECInstanceId psaInstanceId;
    ECInstanceId pInstanceId;
    ECInstanceId th3InstanceId;
    ECInstanceId thBaseInstanceId;

    {
    Savepoint savepoint(ecdb, "Inserting test instances", true);
    psaInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
    th3InstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.TH3 (S3) VALUES ('Test instance for relationship tests')");
    thBaseInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.THBase (S) VALUES ('Test instance for relationship tests')");

    if (!psaInstanceId.IsValid() || !pInstanceId.IsValid() || !th3InstanceId.IsValid() || !thBaseInstanceId.IsValid())
        {
        savepoint.Cancel();
        return dataset;
        }

    savepoint.Commit();
    }

    //**** Case: source/target class id not mandatory as source/target constraints are unambiguous
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());//111,115
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), psaInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA VALUES (%s, %s, %s, %s);", psaInstanceId.ToString().c_str(),
                     psaClassId.ToString().c_str(), psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA VALUES (%s, :classid, %s, :classid);", psaInstanceId.ToString().c_str(),
                     psaInstanceId.ToString().c_str());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("classid", ECValue(psaClassId.GetValue())));
    }

    //mismatching class ids. ECDb does not validate, so ECSQL doesn't fail.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //mismatching class ids. ECDb does not validate, so ECSQL doesn't fail.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    //mismatching class ids. ECDb does not validate, so ECSQL doesn't fail.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    //mismatching constraint ECInstanceIds. They are caught because of FK constraints.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsqlStr.c_str(), ECSqlExpectedResult::Category::Invalid);
    }

    //mismatching class ids. ECDb does not validate, so ECSQL doesn't fail.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA VALUES (%s, %s, %s, %s);", psaInstanceId.ToString().c_str(),
                     th3ClassId.ToString().c_str(), psaInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //mismatching class ids. ECDb does not validate, so ECSQL doesn't fail.
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaInstanceId.ToString().c_str(), psaInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(th3ClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaInstanceId.ToString().c_str(), thBaseInstanceId.ToString().c_str());

    //Target is non-polymorphic -> target class id not required
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s,?, %s, ?)",
                     psaInstanceId.ToString().c_str(), thBaseInstanceId.ToString().c_str());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    //**** Case: target class id is mandatory as target constraint is ambiguous

    //insert with instances of target constraint base class
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), thBaseInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaInstanceId.ToString().c_str(), thBaseInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), thBaseInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //insert with instances of target constraint sub class
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), th3InstanceId.ToString().c_str(), th3ClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(th3ClassId.GetValue())));
    }

    //omit source class id which is not required as source constraint is only one class
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str(), th3ClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), th3InstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //mismatching class ids. ECDb doesn't enforce that, so ECSQL succeeds
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str(), psaClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //mismatching class ids. ECDb doesn't enforce that, so ECSQL succeeds
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    //Target constraint is non-polymorphic, so specifying a subclass id is invalid. But ECDb doesn't enforce that, so ECSQL succeeds
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str(), th3ClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //Target constraint is non-polymorphic, so specifying a subclass id is invalid. But ECDb doesn't enforce that, so ECSQL succeeds
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(th3ClassId.GetValue())));
    }

    //specifying non-sense class id. ECDb does not validate, so ECSQL doesn't fail.
    for (Utf8StringCR nonSenseClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, %s, %s);",
                         psaInstanceId.ToString().c_str(), nonSenseClassIdValue.c_str(), psaInstanceId.ToString().c_str());
        ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);

        ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                         psaInstanceId.ToString().c_str(), pInstanceId.ToString().c_str(), nonSenseClassIdValue.c_str());
        ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);

        ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                         psaInstanceId.ToString().c_str(), nonSenseClassIdValue.c_str(), thBaseInstanceId.ToString().c_str(), thBaseClassId.ToString().c_str());
        ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);

        ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s);",
                         psaInstanceId.ToString().c_str(), th3InstanceId.ToString().c_str(), nonSenseClassIdValue.c_str());
        ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipWithAdditionalPropsTests(ECDbR ecdb)
    {
    ECClassId psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    ECClassId pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");

    ECSqlTestDataset dataset;

    ECInstanceId psaInstanceId;
    ECInstanceId pInstanceId;

    {
    Savepoint savepoint(ecdb, "Inserting test instances", true);
    psaInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");

    if (!psaInstanceId.IsValid() || !pInstanceId.IsValid())
        {
        savepoint.Cancel();
        return dataset;
        }

    savepoint.Commit();
    }

    //Additional props
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s);",
                     psaInstanceId.ToString().c_str(), pInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, B, D, I, S) VALUES (%s, %s, %s, %s, True, 3.14, 123, 'hello');",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), pInstanceId.ToString().c_str(), pClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, %s, %s);",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), pInstanceId.ToString().c_str(), pClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, D, B) VALUES (%s, %s, %s, %s, 3.14, True);",
                     psaInstanceId.ToString().c_str(), psaClassId.ToString().c_str(), pInstanceId.ToString().c_str(), pClassId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipWithParametersTests(ECDbR ecdb)
    {
    ECClassId psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    ECClassId thBaseClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "THBase");

    ECSqlTestDataset dataset;

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId thBaseECInstanceId;
    {
    Savepoint savepoint(ecdb, "Inserting test instances", true);
    psaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
    thBaseECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance(ecdb, "INSERT INTO ecsql.THBase (S) VALUES ('Test instance for relationship tests')");

    if (!psaECInstanceId.IsValid() || !pECInstanceId.IsValid() || !thBaseECInstanceId.IsValid())
        {
        savepoint.Cancel();
        return dataset;
        }

    savepoint.Commit();
    }

    //End table mapping
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(psaECInstanceId));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(thBaseECInstanceId));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (:sourceecinstanceid, :targetecinstanceid);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("sourceecinstanceid", psaECInstanceId));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("targetecinstanceid", thBaseECInstanceId));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%s, :targetecinstanceid);",
                     psaECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("targetecinstanceid", thBaseECInstanceId));
    }

    //Link table mapping
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(psaECInstanceId));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(pECInstanceId));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (:sourceecinstanceid, :targetecinstanceid);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("sourceecinstanceid", psaECInstanceId));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue("targetecinstanceid", pECInstanceId));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId, B, D) VALUES (?, %s, ?, ?);",
                     pECInstanceId.ToString().c_str());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(psaECInstanceId));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(false)));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(3.1415)));
    }

    //**** Parametrized ClassId
    //End table mapping - class id optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    //End table mapping - class id not optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    {
    //mandatory class id is not enforced by ECDb
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    {
    //mandatory class id is not enforced by ECDb
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    //mandatory class id is not enforced by ECDb
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue()));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    //Link table mapping - class id optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    }

    {
    //mandatory class id is not enforced by ECDb
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%s, ?, %s);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);   
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%s, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }

    {
    //mandatory class id is not enforced by ECDb
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    }

    //Link table mapping - class id not optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%s, ?, %s, ?);",
                     psaECInstanceId.ToString().c_str(), thBaseECInstanceId.ToString().c_str());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsqlStr.c_str(), true);
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(psaClassId.GetValue())));
    testItem.AddParameterValue(ECSqlTestItem::ParameterValue(ECValue(thBaseClassId.GetValue())));
    }


    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::StructTests()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp, B) VALUES (NULL, true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStructProp, B) VALUES (?, true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStructProp.i, B) VALUES (123, true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStructProp.i, PStructProp.dt, B) VALUES (123, DATE '2010-10-10', true)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (?)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp.i, SAStructProp.PStructProp.dt) VALUES (123, DATE '2010-10-10')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    return dataset;
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE