/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlInsertTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include "ECDbTestHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, Insert)
    {
            {
            ECTEST_SETUP("ECSqlStatementTests", "ECSqlStatementTests.01.00.ecschema.xml", L"Insert.ecdb");

            STATEMENT_PREPARE_SUCCESS("INSERT INTO ECST.Product(ProductAvailable,ProductName,UnitPrice) VALUES(?,?,?)");
            BIND_BOOLEAN(1, true);
            BIND_TEXT(2, "Hoodie", IECSqlBinder::MakeCopy::No);
            BIND_DOUBLE(3, 350.56);
            STATEMENT_EXECUTE_SUCCESS();

            STATEMENT_PREPARE_SUCCESS("INSERT INTO ECST.Supplier(ContactName,Country) VALUES('John Snow','England')");
            STATEMENT_EXECUTE_SUCCESS();

            EXPECT_STATEMENT_PREPARE("INSERT INTO ECST.Orders(ShipCity) VALUES('NewYork')", ECSqlStatus::InvalidECSql);
            }

            {
            ECTEST_SETUP("Test2", "NestedStructArrayTest.01.00.ecschema.xml", L"Insert1.ecdb");

            STATEMENT_PREPARE_SUCCESS("INSERT INTO nsat.ClassA(I,T) VALUES(8,'testVal')");
            STATEMENT_EXECUTE_SUCCESS();
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, Update)
    {
    ECTEST_SETUP("ECSqlStatementTests", "ECSqlStatementTests.01.00.ecschema.xml", L"Update.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ECST.Supplier(ContactName,Country) VALUES('John Snow','England')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("UPDATE ECST.Supplier SET ContactName = 'Micheal' WHERE ContactName = 'John Snow'");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("UPDATE ECST.Supplier SET ContactName = 'Snape' WHERE ContactName = ?");
    BIND_TEXT(1, "Micheal", IECSqlBinder::MakeCopy::No);
    STATEMENT_EXECUTE_SUCCESS();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, Delete)
    {
    ECTEST_SETUP("ECSqlStatementTests", "NestedStructArrayTest.01.00.ecschema.xml", L"Delete.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO nsat.ClassA(I,T) VALUES(8,'testVal')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("DELETE FROM nsat.ClassA WHERE I=?");
    BIND_INT(1, 8);
    STATEMENT_EXECUTE_SUCCESS();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, ParameterAdvancedTests)
    {
    ECTEST_SETUP("ParameterAdvancedTests", "ECSqlTest.01.00.ecschema.xml", L"ParameterAdvancedTests.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, S) VALUES (123, ?)");
    BIND_TEXT(1, "hello", IECSqlBinder::MakeCopy::No);
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, S) VALUES (?, ?)");
    BIND_INT(1, 123);
    BIND_TEXT(2, "hello", IECSqlBinder::MakeCopy::No);
    STATEMENT_EXECUTE_SUCCESS();

    //Blob
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, Bi) VALUES (123, ?)");
    Byte blob[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00};
    BIND_BINARY(1, blob, 10, IECSqlBinder::MakeCopy::No);
    STATEMENT_EXECUTE_SUCCESS();

    //Points
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, P2D, P3D) VALUES (123, ?, ?)");
    BIND_POINT2D(1, DPoint2d::From(1.1, 2.2));
    BIND_POINT3D(2, DPoint3d::From(1.1, 2.2, 3.3));
    STATEMENT_EXECUTE_SUCCESS();

    //Null
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, S) VALUES (?, ?)");
    BIND_INT(1, 123);
    BIND_NULL(2);
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, S) VALUES (?, ?)");
    BIND_NULL(1);
    BIND_NULL(2);
    STATEMENT_EXECUTE_SUCCESS();

    //reusing named parameter
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Bi, I, S, L, B) VALUES (?, ?, ?, ?, ?)");
    Byte Blob[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00};
    BIND_BINARY(1, Blob, 10, IECSqlBinder::MakeCopy::No);
    BIND_INT(2, 123);
    BIND_TEXT(3, "hello", IECSqlBinder::MakeCopy::No);
    BIND_INT(4, 123);
    BIND_BOOLEAN(5, true);
    STATEMENT_EXECUTE_SUCCESS();

    //Date time <-> basic primitive types
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt) VALUES (?)");
    BIND_INT_STATUS(1, 1, ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (I) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt) VALUES (?)");
    BIND_DOUBLE_STATUS(1, 1.1, ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (D) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    //Date time <-> Point2D
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt) VALUES (?)");
    BIND_POINT2D_STATUS(1, DPoint2d::From(1.1, 2.2), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P2D) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    //Date time <-> Point3D
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt) VALUES (?)");
    BIND_POINT3D_STATUS(1, DPoint3d::From(1.1, 2.2, 3.3), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P3D) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    //Date time <-> Arrays
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt_Array) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    //Point2D <-> basic primitive types
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P2D) VALUES (?)");
    BIND_DOUBLE_STATUS(1, 1.1, ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (D) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(2013, 1, 1), ECSqlStatus::Error);

    //Point2D <-> P3D
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P2D) VALUES (?)");
    BIND_POINT3D_STATUS(1, DPoint3d::From(1.1, 2.2, 3.3), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P3D) VALUES (?)");
    BIND_POINT2D_STATUS(1, DPoint2d::From(1.1, 2.2), ECSqlStatus::Error);

    //Point2D <-> Structs
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStructProp) VALUES (?)");
    BIND_POINT2D_STATUS(1, DPoint2d::From(1.1, 2.2), ECSqlStatus::Error);

    //Point2D <-> Arrays
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P2D_Array) VALUES (?)");
    BIND_POINT2D_STATUS(1, DPoint2d::From(1.1, 2.2), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)");
    BIND_POINT2D_STATUS(1, DPoint2d::From(1.1, 2.2), ECSqlStatus::Error);

    //Point3D <-> Structs
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStructProp) VALUES (?)");
    BIND_POINT3D_STATUS(1, DPoint3d::From(1.1, 2.2, 3.3), ECSqlStatus::Error);

    //Point3D <-> Arrays
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (P3D_Array) VALUES (?)");
    BIND_POINT3D_STATUS(1, DPoint3d::From(1.1, 2.2, 3.3), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)");
    BIND_POINT3D_STATUS(1, DPoint3d::From(1.1, 2.2, 3.3), ECSqlStatus::Error);

    //structs <-> primitives
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStructProp) VALUES (?)");
    BIND_INT_STATUS(1, 123, ECSqlStatus::Error);

    //arrays <-> primitives
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (I_Array) VALUES (?)");
    BIND_INT_STATUS(1, 123, ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStruct_Array) VALUES (?)");
    BIND_INT_STATUS(1, 1, ECSqlStatus::Error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, ArrayTests)
    {
    ECTEST_SETUP("ArrayTests", "ECSqlTest.01.00.ecschema.xml", L"ArrayTests.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt_Array, B) VALUES (NULL, true)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (PStruct_Array, B) VALUES (NULL, true)");
    STATEMENT_EXECUTE_SUCCESS();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, CommonGeometryTests)
    {
    ECTEST_SETUP("CommonGeometryTests", "ECSqlTest.01.00.ecschema.xml", L"CommonGeometryTests.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, NULL)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, NULL)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PASpatial VALUES (False, 3.14, 123, 'hello', NULL, NULL)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, ?)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)");
    IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    BIND_GEOMETRY(1, *line);
    STATEMENT_EXECUTE_SUCCESS();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, DateTimeTests)
    {
    ECTEST_SETUP("DateTimeTests", "ECSqlTest.01.00.ecschema.xml", L"DateTimeTests.ecdb");

    //Inserting into date time prop without DateTimeInfo CA
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123456')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2013-02-18T06:00:00.000')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55Z')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (DATE '2012-01-18')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (NULL)");
    STATEMENT_EXECUTE_SUCCESS();

    //Inserting into UTC date time prop
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')");
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00')", ECSqlStatus::InvalidECSql);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (DATE '2012-01-18')");
    STATEMENT_EXECUTE_SUCCESS();

    //Inserting into date time prop with DateTimeInfo CA where kind is set to Unspecified
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (DATE '2012-01-18')");
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')", ECSqlStatus::InvalidECSql);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUnspec) VALUES (DATE '2012-01-18')");
    STATEMENT_EXECUTE_SUCCESS();

    //Inserting into date time props with DateTimeInfo CA where component is set to Date-onlys
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (DATE '2013-02-18')");
    STATEMENT_EXECUTE_SUCCESS();

    //DateOnly can take time stamps, too
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00')");
    STATEMENT_EXECUTE_SUCCESS();

    //CURRENT_XXX functions
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (CURRENT_DATE)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_DATE)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIMESTAMP)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_TIMESTAMP)");
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (DtUnspec) VALUES (CURRENT_TIMESTAMP)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIME)", ECSqlStatus::InvalidECSql);

    //*** Parameters ****
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, Dt, DtUtc, DtUnspec, DateOnly) VALUES (123, ?, ?, ?, ?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0));
    BIND_DATETIME(2, DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0));
    BIND_DATETIME(3, DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0));
    BIND_DATETIME(4, DateTime(2013, 2, 18));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (Dt) VALUES (?)");
    BIND_DATETIME(1, DateTime(2013, 2, 18));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUtc) VALUES (?)");
    BIND_DATETIME(1, DateTime(2013, 2, 18));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUnspec) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUnspec) VALUES (?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUnspec) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DtUnspec) VALUES (?)");
    BIND_DATETIME(1, DateTime(2013, 2, 18));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Utc, 2013, 2, 18, 6, 0, 0));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (?)");
    BIND_DATETIME(1, DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 6, 0, 0));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (?)");
    BIND_DATETIME_STATUS(1, DateTime(DateTime::Kind::Local, 2013, 2, 18, 6, 0, 0), ECSqlStatus::Error);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (DateOnly) VALUES (?)");
    BIND_DATETIME(1, DateTime(2013, 2, 18));
    STATEMENT_EXECUTE_SUCCESS();
    }

TEST(ECSqlInsertTests, FunctionTests)
    {
    ECTEST_SETUP("FunctionTests", "ECSqlTest.01.00.ecschema.xml", L"FunctionTests.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, L) VALUES (123, GetECClassId ())");
    STATEMENT_EXECUTE_SUCCESS();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, IntoTests)
    {
    ECTEST_SETUP("IntoTests", "ECSqlTest.01.00.ecschema.xml", L"IntoTests.ecdb");

    //******************************************************************
    // Inserting into classes which map to tables with ECClassId columns
    //******************************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.THBase (S) VALUES ('hello')");
    STATEMENT_EXECUTE_SUCCESS();

    //inserting into classes with base classes
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.TH5 (S, S1, S3, S5) VALUES ('hello', 'hello1', 'hello3', 'hello5')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.TH3 VALUES ('hello', 'hello1', 'hello2', 'hello3')");
    STATEMENT_EXECUTE_SUCCESS();

    //*******************************************************
    // Inserting into structs
    //*******************************************************

    //structs are not insertible
    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PStruct (i, l, dt, b) VALUES (123, 1000000, DATE '2013-10-10', False)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PStruct VALUES (False, NULL, 3.14, DATE '2013-10-10', TIMESTAMP '2013-10-10T12:00Z', 123, 10000000, 'hello', NULL, NULL)", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Inserting into CAs
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("INSERT INTO bsca.DateTimeInfo (DateTimeKind) VALUES ('Utc')", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PUnmapped (I, D) VALUES (123, 3.14)", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Abstract classes
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.Abstract (I, S) VALUES (123, 'hello')", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.Abstract NoSubclasses (I, S) VALUES (123, 'hello')", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Subclasses of abstract class
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.Sub1 (I, S, Sub1I) VALUES (123, 'hello', 100123)");
    STATEMENT_EXECUTE_SUCCESS();

    //*******************************************************
    // Empty classes
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.Empty (ECInstanceId) VALUES (NULL)");
    STATEMENT_EXECUTE_SUCCESS();

    //*******************************************************
    // Unsupported classes
    //*******************************************************
    //AnyClass is unsupported, but doesn't have properties, so it cannot be used in an INSERT statement because of that in the first place
    ASSERT_STATEMENT_PREPARE("INSERT INTO bsm.InstanceCount (ECSchemaName, ECClassName, Count) VALUES ('Foo', 'Goo', 103)", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("INSERT INTO PSA (I, L, Dt) VALUES (123, 1000000, DATE '2013-10-10')", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.BlaBla VALUES (123)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO blabla.PSA VALUES (123)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA (Garbage, I, L) VALUES ('bla', 123, 100000000)", ECSqlStatus::InvalidECSql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, MiscTests)
    {
    ECTEST_SETUP("MiscTests", "ECSqlTest.01.00.ecschema.xml", L"MiscTests.ecdb");

    //*******************************************************
    // Syntactically incorrect statements 
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT ecsql.P (I) VALUES (123)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (I)", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Insert expressions 
    //*******************************************************
    ADD_QUERY("INSERT INTO ecsql.P (I) VALUES (1 + 1)");
    ADD_QUERY("INSERT INTO ecsql.P (I) VALUES (5 * 4)");
    ADD_QUERY("INSERT INTO ecsql.P (L) VALUES (1 + ECClassId)");
    ADD_QUERY("INSERT INTO ecsql.P (L) VALUES (1 + GetECClassId())");
    ADD_QUERY("INSERT INTO ecsql.P (L) VALUES (ECClassId * 4)");
    ADD_QUERY("INSERT INTO ecsql.P (L) VALUES (GetECClassId() * 4)");

    //*******************************************************
    // Insert ECInstanceId 
    //*******************************************************
    //NULL for ECInstanceId means ECDb auto-generates the ECInstanceId.
    ADD_QUERY("INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)");
    ADD_QUERY("INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (NULL)");//table per hierarchy mapping->class id must be populated
    ADD_QUERY("INSERT INTO ecsql.P (ECInstanceId, I) VALUES (NULL, NULL)");
    ADD_QUERY("INSERT INTO ecsql.TH2 (ECInstanceId, S2) VALUES (NULL, 'hello')");//table per hierarchy mapping->class id must be populated
    ADD_QUERY("INSERT INTO ecsql.P (ECInstanceId) VALUES (123)");
    ADD_QUERY("INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (4443412341)");//table per hierarchy mapping->class id must be populated

    EXECUTE_LIST();

    //If ECInstanceId is specified via parameter, parameter must be bound.
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (ECInstanceId) VALUES (?)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (ECInstanceId) VALUES (?)");
    BIND_ID(1, ECInstanceId(UINT64_C(141231498)));
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (?)");//table per hierarchy mapping->class id must be populated
    BIND_ID(1, ECInstanceId(UINT64_C(141231498)));
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSAHasP (ECInstanceId) VALUES (NULL)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSAHasPSA (ECInstanceId) VALUES (NULL)", ECSqlStatus::InvalidECSql);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.TH2 (ECInstanceId, S1) VALUES (41241231231, 's1')");//table per hierarchy mapping->class id must be populated;
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (ECInstanceId) VALUES (123)");
    ASSERT_STATEMENT_EXECUTE(DbResult::BE_SQLITE_CONSTRAINT_PRIMARYKEY);   //provoke pk constraint violation as instance with id 123 already exists

    //*******************************************************
    // Class aliases
    //*******************************************************
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them during preparation

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA t (t.Dt, t.L, t.S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Insert literal values
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt, L, S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (L, S, I) VALUES (100000000000, 'hello, \" world', -1)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (L, I) VALUES (CAST (100000 AS INT64), 12 + 99)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (L, S, DtUtc) VALUES (?, ?, ?)");
    STATEMENT_EXECUTE_SUCCESS();

    //*******************************************************
    // Insert without column clause
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL, NULL)");
    STATEMENT_EXECUTE_SUCCESS();

    //*******************************************************
    //  Type conversions
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (I, L) VALUES ('bla', 123, 100000000)", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (I, L) VALUES (123)", ECSqlStatus::InvalidECSql);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.P (I, L) VALUES (123, DATE '2013-04-30')");
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P VALUES (123, 'bla bla')", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL)", ECSqlStatus::InvalidECSql);

    //*******************************************************
    //  Literals
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (B) VALUES (true)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (B) VALUES (True)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (B) VALUES (false)");
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA (B) VALUES (UNKNOWN)", ECSqlStatus::InvalidECSql);

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt) VALUES (DATE '2012-01-18')");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA (Dt) VALUES (TIMESTAMP '2012-01-18T13:02:55')");
    STATEMENT_EXECUTE_SUCCESS();

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA (Dt) VALUES (TIME '13:35:16')", ECSqlStatus::InvalidECSql); //"TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects."

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA (Dt) VALUES (LOCALTIME)", ECSqlStatus::InvalidECSql); // "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA (P2D) VALUES (POINT2D (-1.3, 45.134))", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.PSA (P3D) VALUES (POINT3D (-1.3, 45.134, 2))", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Not yet supported flavors
    //*******************************************************
    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (I, L) SELECT I, L FROM ecsql.PSA", ECSqlStatus::InvalidECSql);

    ASSERT_STATEMENT_PREPARE("INSERT INTO ecsql.P (I, L) VALUES (1, 1234), (2, 32434)", ECSqlStatus::InvalidECSql);

    //*******************************************************
    // Insert clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql._UnderBar (_A_B_C, _ABC, _ABC_, A_B_C_, ABC_) VALUES ('_A_B_C', 2, '_ABC_', 4, 'ABC_')");
    STATEMENT_EXECUTE_SUCCESS();

    //insert clause with square brackets
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.[_UnderBar] ([_A_B_C], [_ABC], [_ABC_], [A_B_C_], [ABC_]) VALUES ('_A_B_C', 2, '_ABC_', 4, 'ABC_')");
    STATEMENT_EXECUTE_SUCCESS();

    //*******************************************************
    // Insert query where string literal consists of Escaping single quotes
    //*******************************************************
    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql._UnderBar (_A_B_C, _ABC, _ABC_, A_B_C_, ABC_) VALUES ('5''55''', 2, '''_ABC_', 4, 'ABC_''')");
    STATEMENT_EXECUTE_SUCCESS();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlInsertTests, StructTests)
    {
    ECTEST_SETUP("StructTests", "ECSqlTest.01.00.ecschema.xml", L"StructTests.ecdb");

    ADD_QUERY("INSERT INTO ecsql.PSA (PStructProp, B) VALUES (NULL, true)");
    ADD_QUERY("INSERT INTO ecsql.PSA (PStructProp, B) VALUES (?, true)");
    ADD_QUERY("INSERT INTO ecsql.PSA (PStructProp.i, B) VALUES (123, true)");
    ADD_QUERY("INSERT INTO ecsql.PSA (PStructProp.i, PStructProp.dt, B) VALUES (123, DATE '2010-10-10', true)");
    ADD_QUERY("INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (NULL)");
    ADD_QUERY("INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (?)");
    ADD_QUERY("INSERT INTO ecsql.SA (SAStructProp.PStructProp.i, SAStructProp.PStructProp.dt) VALUES (123, DATE '2010-10-10')");

    EXECUTE_LIST();
    }

TEST(ECSqlInsertTests, NamedParameterTest)
    {
    ECTEST_SETUP("NamedParameterTest", "ECSqlTest.01.00.ecschema.xml", L"NamedParameterTest.ecdb");

    STATEMENT_PREPARE_SUCCESS("INSERT INTO ecsql.PSA(S,B) VALUES ('lalala', true)");
    STATEMENT_EXECUTE_SUCCESS();

    STATEMENT_PREPARE_SUCCESS("SELECT S,B FROM ecsql.PSA WHERE S = :s AND B=:b");
    BIND_TEXT("s", "lalala", IECSqlBinder::MakeCopy::No);
    BIND_BOOLEAN("b", true);
    ASSERT_STATEMENT_EXECUTE(DbResult::BE_SQLITE_ROW);
    ASSERT_TEXT(0, "lalala");
    ASSERT_BOOLEAN(1, true);
    }
END_ECDBUNITTESTS_NAMESPACE