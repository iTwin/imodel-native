/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlInsertStatementCrudTestDataset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlInsertStatementCrudTestDataset.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
const std::vector<Utf8String> ECSqlInsertTestDataset::s_invalidClassIdValues = { Utf8String ("NULL"), Utf8String ("'bla'"), Utf8String ("3.14"), Utf8String ("True"), Utf8String ("DATE '2013-10-10'") };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::ArrayTests ()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt_Array, B) VALUES (NULL, true)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStruct_Array, B) VALUES (NULL, true)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::CommonGeometryTests ()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PASpatial VALUES (False, 3.14, 123, 'hello', NULL, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

        {
        ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        IGeometryPtr line = IGeometry::Create (ICurvePrimitive::CreateLine (DSegment3d::From (0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (line));
        }

    ecsql = "INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, ?)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::DateTimeTests ()
    {
    ECSqlTestDataset dataset;

    //Inserting into date time prop without DateTimeInfo CA

    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123456')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2013-02-18T06:00:00.000')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55Z')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //Inserting into UTC date time prop
    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUtc is UTC time stamp while value is not.");

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //Inserting into date time prop with DateTimeInfo CA where kind is set to Unspecified
    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.");

    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //Inserting into date time props with DateTimeInfo CA where component is set to Date-onlys
    ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (DATE '2013-02-18')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //DateOnly can take time stamps, too
    ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //CURRENT_XXX functions
    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (CURRENT_DATE)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_DATE)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIMESTAMP)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_TIMESTAMP)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (CURRENT_TIMESTAMP)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp");

    ecsql = "INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIME)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.");

    //*** Parameters ****
    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (I, Dt, DtUtc, DtUnspec, DateOnly) VALUES (123, ?, ?, ?, ?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (Dt) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }



    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUtc) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }


    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DtUnspec) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }


    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Local time not supported by ECSQL.");
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.P (DateOnly) VALUES (?)";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 2, 18)));
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::FunctionTests ()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.P (I, L) VALUES (123, GetECClassId ())";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::IntoTests ()
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    // Inserting into classes which map to tables with ECClassId columns
    //*******************************************************
    Utf8CP ecsql = "INSERT INTO ecsql.THBase (S) VALUES ('hello')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //inserting into classes with base classes
    ecsql = "INSERT INTO ecsql.TH5 (S, S1, S3, S5) VALUES ('hello', 'hello1', 'hello3', 'hello5')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.TH3 VALUES ('hello', 'hello1', 'hello2', 'hello3')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //*******************************************************
    // Inserting into structs
    //*******************************************************
    //structs which are domain classes are insertible
    ecsql = "INSERT INTO ecsql.SAStruct (PStructProp.i, PStructProp.l, PStructProp.dt, PStructProp.b) VALUES (123, 1000000, DATE '2013-10-10', False)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //structs which are not domain classes are not insertible
    ecsql = "INSERT INTO ecsql.PStruct (i, l, dt, b) VALUES (123, 1000000, DATE '2013-10-10', False)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.PStruct VALUES (False, NULL, 3.14, DATE '2013-10-10', TIMESTAMP '2013-10-10T12:00Z', 123, 10000000, 'hello', NULL, NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Inserting into CAs
    //*******************************************************
    ecsql = "INSERT INTO bsca.DateTimeInfo (DateTimeKind) VALUES ('Utc')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Custom Attributes classes are invalid in INSERT statements.");


    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "INSERT INTO ecsql.PUnmapped (I, D) VALUES (123, 3.14)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in INSERT statements.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    ecsql = "INSERT INTO ecsql.Abstract (I, S) VALUES (123, 'hello')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Abstract classes cannot be used in INSERT statements.");

    ecsql = "INSERT INTO ecsql.AbstractNoSubclasses (I, S) VALUES (123, 'hello')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Abstract classes cannot be used in INSERT statements.");

    //*******************************************************
    // Subclasses of abstract class
    //*******************************************************
    ecsql = "INSERT INTO ecsql.Sub1 (I, S, Sub1I) VALUES (123, 'hello', 100123)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //*******************************************************
    // Empty classes
    //*******************************************************
    ecsql = "INSERT INTO ecsql.Empty (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    //*******************************************************
    // Unsupported classes
    //*******************************************************
    //AnyClass is unsupported, but doesn't have properties, so it cannot be used in an INSERT statement because of that in the first place

    ecsql = "INSERT INTO bsm.InstanceCount (ECSchemaName, ECClassName, Count) VALUES ('Foo', 'Goo', 103)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "INSERT INTO PSA (I, L, Dt) VALUES (123, 1000000, DATE '2013-10-10')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema prefix.");

    ecsql = "INSERT INTO ecsql.BlaBla VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO blabla.PSA VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.PSA (Garbage, I, L) VALUES ('bla', 123, 100000000)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "One of the properties does not exist in the target class.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::MiscTests (ECDbR ecdb)
    {
    ECSqlTestDataset dataset;

    ECInstanceId pECInstanceId;

        {
        Savepoint savepoint (ecdb, "Inserting test instances");
        pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
        if (!pECInstanceId.IsValid ())
            {
            savepoint.Cancel ();
            return dataset;
            }

        savepoint.Commit ();
        }

    //*******************************************************
    // Syntactically incorrect statements 
    //*******************************************************
    Utf8CP ecsql = "";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT ecsql.P (I) VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (I)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Insert expressions 
    //*******************************************************
    ecsql = "INSERT INTO ecsql.P (I) VALUES (1 + 1)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (I) VALUES (5 * 4)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (L) VALUES (1 + GetECClassId())";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (L) VALUES (GetECClassId() * 4)";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    //*******************************************************
    // Insert ECInstanceId 
    //*******************************************************
    //NULL for ECInstanceId means ECDb auto-generates the ECInstanceId.
    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (NULL)"; //table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (ECInstanceId, I) VALUES (NULL, NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId, S2) VALUES (NULL, 'hello')"; //table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (123)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (4443412341)"; //table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //If ECInstanceId is specified via parameter, parameter must be bound.
    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (?)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

        {
        ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECInstanceId (141231498LL)));
        }

        {
        ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (?)";//table per hierarchy mapping->class id must be populated
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECInstanceId (141231498LL)));
        }

    ecsql = "INSERT INTO ecsql.PSAHasP (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.PSAHasPSA (ECInstanceId) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (ECInstanceId, I) VALUES (123, 123)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId, S1) VALUES (41241231231, 's1')";//table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //for link table mappings specifying the ECInstanceId is same as for regular classes
        {
        ecsql = "INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (NULL, 124, 124)";
        ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        }

        {
        ecsql = "INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (123, 124, 124)";
        ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        }

        {
        ecsql = "INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (?, 124, 123)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECInstanceId (123)));
        }


    //for end table mappings specifying the ECInstanceId is a no-op. It will be ignored.
        {
        ecsql = "INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (NULL, 124, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (pECInstanceId));
        }

        {
        ecsql = "INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (?, 124, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (pECInstanceId));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (pECInstanceId));
        }

        {
        ecsql = "INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (14123123, 124, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (pECInstanceId));
        }

    ecsql = "INSERT INTO ecsql.P (ECInstanceId) VALUES (123)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, false);
    //provoke pk constraint violation as instance with id 123 already exists
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (412313)";//table per hierarchy mapping->class id must be populated
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, false);
    //provoke pk constraint violation as instance with id 412313 already exists
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Class aliases
    //*******************************************************
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them
    //during preparation
    ecsql = "INSERT INTO ecsql.PSA t (t.Dt, t.L, t.S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

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
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (I, L) VALUES (123)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P (I, L) VALUES (123, DATE '2013-04-30')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P VALUES (123, 'bla bla')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    
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
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (DATE '2012-01-18')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (TIMESTAMP '2012-01-18T13:02:55')";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (TIME '13:35:16')";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

    ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (LOCALTIME)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

    ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (POINT2D (-1.3, 45.134))";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "INSERT INTO ecsql.PSA (P3D) VALUES (POINT3D (-1.3, 45.134, 2))";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    //*******************************************************
    // Not yet supported flavors
    //*******************************************************
    ecsql = "INSERT INTO ecsql.P (I, L) SELECT I, L FROM ecsql.PSA";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

    ecsql = "INSERT INTO ecsql.P (I, L) VALUES (1, 1234), (2, 32434)";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::NotYetSupported);

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
ECSqlTestDataset ECSqlInsertTestDataset::ParameterAdvancedTests ()
    {
    //This includes only advanced parameter tests that are not covered implicitly by the other test datasets

    ECSqlTestDataset dataset;

        {
        Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (123, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
        }

        //Blob
        {
        Utf8CP ecsql = "INSERT INTO ecsql.P (I, Bi) VALUES (123, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        Byte blob[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00 };
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (blob, 10)));
        }

        //Points
        {
        Utf8CP ecsql = "INSERT INTO ecsql.P (I, P2D, P3D) VALUES (123, ?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
        }

        //binding null
        {
        Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        //bind null. 
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.P (I, S) VALUES (?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        //bind null. 
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
        }
        
        //reusing named parameter
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (I, S, L) VALUES (:i, :s, (:i - 23) * 100)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("i", ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("hello")));
        }

        //mixing unnamed and named parameters
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (I, S, L) VALUES (?, :s, :l)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("s", ECValue ("hello")));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("l", ECValue (INT64_C(123456789))));
        }

    //type match tests
        {
        //primitive types except for points and date times are convertible into each other (by SQLite)
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (Bi, I, S, L, B) VALUES (?, ?, ?, ?, ?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (true)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("hello")));
        Byte blob[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00 };
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (blob, 10)));
        }

        //Date time <-> basic primitive types
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (I) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013,1,1)));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1.1)));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        //Date time <-> Point2D
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        //Date time <-> Point3D
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P3D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        //Date time <-> Structs
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        //Date time <-> Arrays
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (Dt_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        //Point2D <-> basic primitive types
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1.1)));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (DateTime (2013, 1, 1)));
        }

        //Point2D <-> P3D
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P3D) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
        }

        //Point2D <-> Structs
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
        }

        //Point2D <-> Arrays
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P2D_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint2d::From (1.1, 2.2))));
        }

        //Point3D <-> Structs
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
        }

        //Point3D <-> Arrays
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (P3D_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DPoint3d::From (1.1, 2.2, 3.3))));
        }

        //structs <-> primitives
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        //arrays <-> primitives
        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (I_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (123)));
        }

        {
        Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)";
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipEndTableMappingTests (ECDbR ecdb)
    {
    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    auto pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");
    auto thBaseClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "THBase");
    auto th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");

    ECSqlTestDataset dataset;

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId p2ECInstanceId;
    ECInstanceId thBaseECInstanceId;
    ECInstanceId th3ECInstanceId;

        {
        Savepoint savepoint (ecdb, "Inserting test instances", true);
        psaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
        pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
        p2ECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (200, 'Test instance for relationship tests')");
        thBaseECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.THBase (S) VALUES ('Test instance for relationship tests')");
        th3ECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.TH3 (S3) VALUES ('Test instance for relationship tests')");

        Utf8String testRelationshipECSql;
        testRelationshipECSql.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld)",
            psaECInstanceId.GetValue (), p2ECInstanceId.GetValue ());

        auto relECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, testRelationshipECSql.c_str ());

        if (!psaECInstanceId.IsValid () || !pECInstanceId.IsValid () || !p2ECInstanceId.IsValid () || 
            !thBaseECInstanceId.IsValid () || !th3ECInstanceId.IsValid () || !relECInstanceId.IsValid ())
            {
            savepoint.Cancel ();
            return dataset;
            }

        savepoint.Commit ();
        }


    //**** Case: source/target class id not mandatory as source/target constraints are unambiguous
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    //For end table mappings only this end's ecinstance id must match. The other one is not enforced.
    //this end: P
    {
    //this end is correct -> expects success
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
        thBaseECInstanceId.GetValue (), pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    //this end is incorrect -> expects failure
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    //This end already has a relationship of this type -> expects failure
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
    psaECInstanceId.GetValue (), p2ECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), psaClassId, pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, ?, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    //Not binding mandatory class id should provoke a constraint violation
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, ?, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue (), pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (pClassId)));
    }

    {
    //Not binding mandatory class id should provoke a constraint violation
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld, %lld);",
        psaECInstanceId.GetValue (), psaClassId, pECInstanceId.GetValue (), pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    //Target is non-polymorphic, so only THBase items are allowed -> class id should not be necessary
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), th3ECInstanceId.GetValue (), th3ClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }


    //specifying mismatching class ids
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue (), psaClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), pClassId, pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP VALUES (%lld, %lld, %lld, %lld);",
        psaECInstanceId.GetValue (), pClassId, pECInstanceId.GetValue (), psaClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    //specifying invalid class ids
    for (Utf8StringCR invalidClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %s, %lld);",
            psaECInstanceId.GetValue (), invalidClassIdValue.c_str (), pECInstanceId.GetValue ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);

        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %s);",
            psaECInstanceId.GetValue (), pECInstanceId.GetValue (), invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
        }

    //**** Case:  target class id mandatory as target constraint is ambiguous
    
    //Now target is polymorphic, which means that THBase and all subclasses are valid -> target class id is necessary (but source class id not)
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld, %lld);",
        psaECInstanceId.GetValue (), psaClassId, thBaseECInstanceId.GetValue (), thBaseClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    //Not binding mandatory class id should provoke a constraint violation
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue (), thBaseClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    //Affan: This test fail because of cardinality volition
    {
    //Utf8String ecsqlStr;
    //ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
     //   psaECInstanceId.GetValue (), thBaseClassId);
    //ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }


    //specifying mismatching class ids
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue (), pClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (pClassId)));
    }

    //specifying invalid class ids
    for (Utf8StringCR invalidClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %s, %lld, %lld);",
            psaECInstanceId.GetValue (), invalidClassIdValue.c_str (), thBaseECInstanceId.GetValue (), thBaseClassId);
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);

        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %s);",
            psaECInstanceId.GetValue (), pECInstanceId.GetValue (), invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipLinkTableMappingTests (ECDbCR ecdb)
    {
    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    auto thBaseClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "THBase");
    auto th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");

    ECSqlTestDataset dataset;

    //**** Case: source/target class id not mandatory as source/target constraints are unambiguous
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES (200, 234);";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (201, 234, %lld);",
                        psaClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (202, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (203, %lld, 234);",
                        psaClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (204, ?, 234);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA VALUES (205, %lld, 234, %lld);",
                        psaClassId, psaClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA VALUES (206, :classid, 234, :classid);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("classid", ECValue (psaClassId)));
    }

    //Target is non-polymorphic -> target class id not required
    ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId) VALUES (207, 234);";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (208,?, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    //specifying mismatching class ids
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (209, 234, %lld);",
                thBaseClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (210, ?, 234);";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (211, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (212, %lld, 234);",
                thBaseClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA VALUES (213, %lld, 234, %lld);",
                th3ClassId, thBaseClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (214, ?, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (th3ClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
        }

    //specifying invalid class ids
    for (Utf8StringCR invalidClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (9000, %s, 234);",
            invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);

        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (9000, 234, %s);",
            invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
        }

    //**** Case: target class id is mandatory as target constraint is ambiguous

    //insert with instances of target constraint base class
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (1000, %lld, 234, %lld);",
                    psaClassId, thBaseClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (1001, ?, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (1002, 234, %lld);",
                    thBaseClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    //insert with instances of target constraint sub class
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (1003, %lld, 234, %lld);",
                    psaClassId, th3ClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (1004, ?, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (th3ClassId)));
    }

    //omit source class id which is not required as source constraint is only one class
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (1005, 234, %lld);",
                    thBaseClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
        }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (1006, %lld, 234);",
            psaClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr = "INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId) VALUES (1007, 234);";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }
    //mismatching class ids
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (1008, 234, %lld);",
                psaClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);;
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (1009, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    //Target constraint is non-polymorphic, so specifying a subclass id is invalid
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (1010, 234, %lld);",
            th3ClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (1011, 234, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (th3ClassId)));
    }

    //specifying invalid class ids
    for (Utf8StringCR invalidClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (1012, %s, 234, %lld);",
            invalidClassIdValue.c_str (), thBaseClassId);
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);

        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (130, 234, %s);",
            invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipWithAnyClassConstraintTests (ECDbR ecdb)
    {
    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    auto pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");
    auto saClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "SA");
    auto th4ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH4");

    ECSqlTestDataset dataset;

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId saECInstanceId;
    ECInstanceId th4ECInstanceId;
        {
        Savepoint savepoint (ecdb, "Inserting test instances", true);
        psaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
        pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
        saECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.SA (SAStructProp.PStructProp.s) VALUES ('Test instance for relationship tests')");
        th4ECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.TH4 (S4) VALUES ('Test instance for relationship tests')");

        if (!psaECInstanceId.IsValid () || !pECInstanceId.IsValid () || !saECInstanceId.IsValid () || !th4ECInstanceId.IsValid ())
            {
            savepoint.Cancel ();
            return dataset;
            }

        savepoint.Commit ();
        }

    //AnyClass as target constraint (0..N)

    //insert with arbitrary class ids for the any class constraint
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
                      "VALUES (%lld, %lld, %lld, %lld);",
                    psaECInstanceId.GetValue (), psaClassId, saECInstanceId.GetValue (), saClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
        "VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), saECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (saClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
                    "VALUES (%lld, %lld, %lld, %lld);",
                    psaECInstanceId.GetValue (), psaClassId, th4ECInstanceId.GetValue (), th4ClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
        "VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), saECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (th4ClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), saECInstanceId.GetValue (), saClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) "
        "VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), saECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (saClassId)));
    }

    //omitting unnecessary source class id
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), th4ECInstanceId.GetValue (), th4ClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    //omitting mandatory target ecclass id
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
        psaECInstanceId.GetValue (), th4ECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), psaClassId, th4ECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    //specifying invalid class ids
    for (Utf8StringCR invalidClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
                          "VALUES (%lld, %s, %lld, %lld);",
                          psaECInstanceId.GetValue (), invalidClassIdValue.c_str (), th4ECInstanceId.GetValue (), th4ClassId);
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);

        ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %s);",
            psaECInstanceId.GetValue (), th4ECInstanceId.GetValue (), invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
        }

    //AnyClass as source constraint (0..1)
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
                      "VALUES (%lld, %lld, %lld, %lld);",
                      saECInstanceId.GetValue (), saClassId, pECInstanceId.GetValue (), pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
                      "VALUES (%lld, %lld, %lld, %lld);",
                      th4ECInstanceId.GetValue (), th4ClassId, pECInstanceId.GetValue (), pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    //omitting unnecessary target class id
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %lld, %lld);",
        th4ECInstanceId.GetValue (), th4ClassId, pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    }

    {
    //omitting unnecessary target class id
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld);",
        th4ECInstanceId.GetValue (), pECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue (), pClassId);
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    //specifying invalid class ids
    for (Utf8StringCR invalidClassIdValue : s_invalidClassIdValues)
        {
        Utf8String ecsqlStr;
        ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %s, %lld);",
            saECInstanceId.GetValue (), invalidClassIdValue.c_str (), pECInstanceId.GetValue ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);

        ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) "
                          "VALUES (%lld, %lld, %lld, %s);",
                          th4ECInstanceId.GetValue (), th4ClassId, pECInstanceId.GetValue (), invalidClassIdValue.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipWithAdditionalPropsTests (ECDbCR ecdb)
    {
    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    auto pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");

    ECSqlTestDataset dataset;

    //Additional props
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (400, 234);";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, B, D, I, S) VALUES (401, %lld, 235, %lld, True, 3.14, 123, 'hello');", psaClassId, pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (402, %lld, 236, %lld);", psaClassId, pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, D, B) VALUES (403, %lld, 237, %lld, 3.14, True);", psaClassId, pClassId);
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str ());
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::RelationshipWithParametersTests (ECDbR ecdb)
    {
    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    auto thBaseClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "THBase");
    auto th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");

    ECSqlTestDataset dataset;

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId thBaseECInstanceId;
    ECInstanceId th3ECInstanceId;
        {
        Savepoint savepoint (ecdb, "Inserting test instances", true);
        psaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
        pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");
        thBaseECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.THBase (S) VALUES ('Test instance for relationship tests')");
        th3ECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.TH3 (S3) VALUES ('Test instance for relationship tests')");

        if (!psaECInstanceId.IsValid () || !pECInstanceId.IsValid () || !thBaseECInstanceId.IsValid () || !th3ECInstanceId.IsValid ())
            {
            savepoint.Cancel ();
            return dataset;
            }

        savepoint.Commit ();
        }

    //End table mapping
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (psaECInstanceId));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (thBaseECInstanceId));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (:sourceecinstanceid, :targetecinstanceid);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("sourceecinstanceid", psaECInstanceId));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("targetecinstanceid", thBaseECInstanceId));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, :targetecinstanceid);",
                    psaECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("targetecinstanceid", thBaseECInstanceId));
    }

    //Link table mapping
    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (psaECInstanceId));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (pECInstanceId));
    }

    {
    Utf8CP ecsql = "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (:sourceecinstanceid, :targetecinstanceid);";
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("sourceecinstanceid", psaECInstanceId));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue ("targetecinstanceid", pECInstanceId));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId, B, D) VALUES (?, %lld, ?, ?);",
                    pECInstanceId.GetValue ());

    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (psaECInstanceId));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (false)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (3.1415)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (?, %lld, %lld);",
        th3ECInstanceId.GetValue (), th3ClassId);
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (psaECInstanceId));
    }

    //**** Parametrized ClassId
    //End table mapping - class id optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, ?, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    //End table mapping - class id not optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    //Must always bind to class id
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    //Must always bind to class id
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    //Must always bind to class id
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ()));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    //Link table mapping - class id optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, ?, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    {
    //Must always bind to ClassId
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, ?, %lld);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }

    {
    //Must always bind to ClassId
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    ECSqlTestFrameworkHelper::AddStepFailingNonSelect (dataset, ecsqlStr.c_str (), ECSqlExpectedResult::Category::Invalid);
    }

    //Link table mapping - class id not optional
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaECInstanceId.GetValue (), thBaseECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (thBaseClassId)));
    }


    //AnyClass constraint
    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, %lld, ?);",
        psaECInstanceId.GetValue (), th3ECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (th3ClassId)));
    }

    {
    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, ?, %lld);",
        psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
    auto& testItem = ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsqlStr.c_str (), true);
    testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (psaClassId)));
    }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlInsertTestDataset::StructTests ()
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "INSERT INTO ecsql.PSA (PStructProp, B) VALUES (NULL, true)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStructProp, B) VALUES (?, true)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStructProp.i, B) VALUES (123, true)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.PSA (PStructProp.i, PStructProp.dt, B) VALUES (123, DATE '2010-10-10', true)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (NULL)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (?)";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp.i, SAStructProp.PStructProp.dt) VALUES (123, DATE '2010-10-10')";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql);

    return dataset;
    }


