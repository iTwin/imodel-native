/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatementTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"
#include "NestedStructArrayTestSchemaHelper.h"
#include <Bentley/Base64Utilities.h>
#include <cmath>
#include <algorithm>
#include <rapidjson/BeRapidJson.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlStatus, Misc)
    {
    ECSqlStatus empty;
    ASSERT_EQ(ECSqlStatus::Status::Success, empty.Get());
    ASSERT_EQ(ECSqlStatus::Status::Success, empty) << "implicit cast operator";
    ASSERT_FALSE(empty.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_OK, empty.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus::Success == empty);
    ASSERT_FALSE(ECSqlStatus::Success != empty);

    ECSqlStatus invalidECSql = ECSqlStatus::InvalidECSql;
    ASSERT_EQ(ECSqlStatus::Status::InvalidECSql, invalidECSql.Get());
    ASSERT_EQ(ECSqlStatus::Status::InvalidECSql, invalidECSql) << "implicit cast operator";
    ASSERT_FALSE(invalidECSql.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_OK, invalidECSql.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus::InvalidECSql == invalidECSql);
    ASSERT_FALSE(ECSqlStatus::InvalidECSql != invalidECSql);

    ECSqlStatus error = ECSqlStatus::Error;
    ASSERT_EQ(ECSqlStatus::Status::Error, error.Get());
    ASSERT_EQ(ECSqlStatus::Status::Error, error) << "implicit cast operator";
    ASSERT_FALSE(error.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_OK, error.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus::Error == error);
    ASSERT_FALSE(ECSqlStatus::Error != error);

    ECSqlStatus sqliteError(BE_SQLITE_CONSTRAINT_CHECK);
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError.Get());
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError) << "implicit cast operator";
    ASSERT_TRUE(sqliteError.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_CHECK, sqliteError.GetSQLiteError());

    ASSERT_TRUE(ECSqlStatus(BE_SQLITE_CONSTRAINT_CHECK) == sqliteError);
    ASSERT_FALSE(ECSqlStatus(BE_SQLITE_CONSTRAINT_CHECK) != sqliteError);

    ECSqlStatus sqliteError2(BE_SQLITE_BUSY);
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError2.Get());
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError2) << "implicit cast operator";
    ASSERT_TRUE(sqliteError2.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_BUSY, sqliteError2.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus(BE_SQLITE_BUSY) == sqliteError2);
    ASSERT_FALSE(ECSqlStatus(BE_SQLITE_BUSY) != sqliteError2);

    ASSERT_FALSE(sqliteError == sqliteError2);
    ASSERT_TRUE(sqliteError != sqliteError2);
    ASSERT_FALSE(sqliteError2 == sqliteError);
    ASSERT_TRUE(sqliteError2 != sqliteError);

    ASSERT_FALSE(empty == invalidECSql);
    ASSERT_TRUE(empty != invalidECSql);

    ASSERT_FALSE(empty == error);
    ASSERT_TRUE(empty != error);

    ASSERT_FALSE(empty == sqliteError);
    ASSERT_TRUE(empty != sqliteError);

    ASSERT_FALSE(empty == sqliteError2);
    ASSERT_TRUE(empty != sqliteError2);

    ASSERT_FALSE(invalidECSql == error);
    ASSERT_TRUE(invalidECSql != error);

    ASSERT_FALSE(invalidECSql == sqliteError);
    ASSERT_TRUE(invalidECSql != sqliteError);

    ASSERT_FALSE(invalidECSql == sqliteError2);
    ASSERT_TRUE(invalidECSql != sqliteError2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct PowSqlFunction : ScalarFunction
    {
    private:

        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
            {
            if (args[0].IsNull() || args[1].IsNull())
                {
                ctx.SetResultError("Arguments to POW must not be NULL", -1);
                return;
                }

            double base = args[0].GetValueDouble();
            double exp = args[1].GetValueDouble();

            double res = std::pow(base, exp);
            ctx.SetResultDouble(res);
            }

    public:
        PowSqlFunction() : ScalarFunction("POW", 2, DbValueType::FloatVal) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, PopulateECSql_TestDbWithTestData)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, UnionTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    int rowCount;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM (SELECT CompanyName FROM ECST.Supplier UNION ALL SELECT CompanyName FROM ECST.Shipper)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    int count = stmt.GetValueInt(0);
    EXPECT_EQ(6, count);
    stmt.Finalize();

    //Select Statement containing Union All Clause and also Order By clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName, Phone FROM ECST.Supplier UNION ALL SELECT CompanyName, Phone FROM ECST.Shipper ORDER BY Phone"));
    rowCount = 0;
    Utf8CP expectedContactNames = "ABCD-Rio Grand-GHIJ-Rio Grand-Rue Perisnon-Salguero-";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(6, rowCount);
    stmt.Finalize();

    //Select Statement using UNION Clause, so we should get only distinct results
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT City FROM ECST.Supplier UNION SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    Utf8CP expectedCityNames = "ALASKA-AUSTIN-CA-MD-NC-SAN JOSE-";
    Utf8String actualCityNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualCityNames.append(stmt.GetValueText(0));
        actualCityNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedCityNames, actualCityNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(6, rowCount);
    stmt.Finalize();

    //Select Statement Using UNION ALL Clause so we should get even Duplicate Results
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT City FROM ECST.Supplier UNION ALL SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    expectedCityNames = "ALASKA-AUSTIN-CA-MD-NC-SAN JOSE-SAN JOSE-";
    actualCityNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualCityNames.append(stmt.GetValueText(0));
        actualCityNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedCityNames, actualCityNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(7, rowCount);
    stmt.Finalize();

    //use Custom Scaler function in union query
    PowSqlFunction func;
    ASSERT_EQ(0, m_ecdb.AddFunction(func));
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "Select POW(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
    rowCount = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        int base = stmt.GetValueInt(2);
        ASSERT_EQ(std::pow(base, 2), stmt.GetValueInt(0));
        rowCount++;
        }
    ASSERT_EQ(7, rowCount);
    stmt.Finalize();

    //use aggregate function in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Count(*), AVG(Phone) FROM (SELECT Phone FROM ECST.Supplier UNION ALL SELECT Phone FROM ECST.Customer)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ(7, stmt.GetValueInt(0));
    ASSERT_EQ(1400, stmt.GetValueInt(1));
    stmt.Finalize();

    //Use GROUP BY clause in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*), Phone FROM (SELECT ECClassId, Phone FROM ECST.Supplier UNION ALL SELECT ECClassId, Phone FROM ECST.Customer) GROUP BY ECClassId ORDER BY Phone"));

    //Get Row one
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(3, stmt.GetValueInt(0));
    ASSERT_EQ(1300, stmt.GetValueDouble(1));

    //Get Row two
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(4, stmt.GetValueInt(0));
    ASSERT_EQ(1700, stmt.GetValueDouble(1));

    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ExceptTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName FROM ECST.Supplier EXCEPT SELECT CompanyName FROM ECST.Shipper"));
    int rowCount = 0;
    Utf8CP expectedContactNames = "ABCD-GHIJ-";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(2, rowCount);

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ContactTitle FROM ECST.Customer EXCEPT SELECT ContactTitle FROM ECST.Supplier"));
    rowCount = 0;
    expectedContactNames = "AM-Adm-SPIELMANN-";
    actualContactNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(3, rowCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, IntersectTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName FROM ECST.Supplier INTERSECT SELECT CompanyName FROM ECST.Shipper ORDER BY CompanyName"));
    int rowCount = 0;
    Utf8CP expectedContactNames = "Rio Grand";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str());
    ASSERT_EQ(1, rowCount);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ContactTitle FROM ECST.Supplier INTERSECT SELECT ContactTitle FROM ECST.Customer ORDER BY ContactTitle"));
    rowCount = 0;
    expectedContactNames = "Brathion";
    actualContactNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str());
    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                          04/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, QuoteTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO stco.ClassWithPrimitiveProperties (stringProp) VALUES('''a''a''')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT stringProp FROM stco.ClassWithPrimitiveProperties WHERE stringProp = '''a''a'''"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ONLY stco.ClassWithPrimitiveProperties SET stringProp = '''g''''g'''"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT stringProp FROM stco.ClassWithPrimitiveProperties WHERE stringProp = '''g''''g'''"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IsNull)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlisnull.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
              <ECStructClass typeName="PrimStruct">
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="p2d" typeName="Point2d" />
                    <ECProperty propertyName="p3d" typeName="Point3d" />
                    <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
              </ECStructClass>
              <ECStructClass typeName="MyStruct">
                    <ECStructProperty propertyName="pstruct" typeName="PrimStruct" />
                    <ECArrayProperty propertyName="l_array" typeName="long" />
                    <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructArrayProperty propertyName="struct_array" typeName="PrimStruct" />
              </ECStructClass>
              <ECEntityClass typeName="Class_NoSharedCols">
                    <ECProperty propertyName="I" typeName="int" />
                    <ECProperty propertyName="L" typeName="long" />
                    <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECProperty propertyName="P2D" typeName="Point2d" />
                    <ECProperty propertyName="P3D" typeName="Point3d" />
                    <ECArrayProperty propertyName="L_Array" typeName="long" />
                    <ECArrayProperty propertyName="Geom_Array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructProperty propertyName="Struct" typeName="MyStruct" />
                    <ECStructArrayProperty propertyName="Struct_Array" typeName="MyStruct" />
             </ECEntityClass>
             <ECEntityClass typeName="Class_SharedCols_NoOverflow">
                   <ECCustomAttributes>
                        <ClassMap xmlns='ECDbMap.02.00'>
                             <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns='ECDbMap.02.00'>
                              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="I" typeName="int" />
                    <ECProperty propertyName="L" typeName="long" />
                    <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECProperty propertyName="P2D" typeName="Point2d" />
                    <ECProperty propertyName="P3D" typeName="Point3d" />
                    <ECArrayProperty propertyName="L_Array" typeName="long" />
                    <ECArrayProperty propertyName="Geom_Array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructProperty propertyName="Struct" typeName="MyStruct" />
                    <ECStructArrayProperty propertyName="Struct_Array" typeName="MyStruct" />
             </ECEntityClass>
             <ECEntityClass typeName="Class_SharedCols_Overflow">
                   <ECCustomAttributes>
                        <ClassMap xmlns='ECDbMap.02.00'>
                             <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns='ECDbMap.02.00'>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="I" typeName="int" />
                    <ECProperty propertyName="L" typeName="long" />
                    <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECProperty propertyName="P2D" typeName="Point2d" />
                    <ECProperty propertyName="P3D" typeName="Point3d" />
                    <ECArrayProperty propertyName="L_Array" typeName="long" />
                    <ECArrayProperty propertyName="Geom_Array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructProperty propertyName="Struct" typeName="MyStruct" />
                    <ECStructArrayProperty propertyName="Struct_Array" typeName="MyStruct" />
             </ECEntityClass>
        </ECSchema>)xml")));

    std::vector<Utf8CP> testClassNames {"Class_NoSharedCols", "Class_SharedCols_NoOverflow", "Class_SharedCols_Overflow"};


    //*** all values null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(ECInstanceId) VALUES(NULL)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct, Struct.struct_array, Struct_Array, "
                      "I, Geom, P2D, P2D.X, P2D.Y, P3D, P3D.X, P3D.Y, P3D.Z, "
                      "Struct.pstruct, Struct.pstruct.i, Struct.pstruct.geom, Struct.pstruct.p2d, Struct.pstruct.p2d.X, Struct.pstruct.p2d.Y, "
                      "Struct.pstruct.p3d, Struct.pstruct.p3d.X, Struct.pstruct.p3d.Y, Struct.pstruct.p3d.Z, "
                      "Struct.l_array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            ASSERT_TRUE(stmt.IsValueNull(i)) << "no values bound to " << stmt.GetECSql();
            }

        const int expectedMembersCount = (int) m_ecdb.Schemas().GetClass("TestSchema", "MyStruct")->GetPropertyCount();
        IECSqlValue const& structVal = stmt.GetValue(0);
        int actualMembersCount = 0;
        for (IECSqlValue const& memberVal : structVal.GetStructIterable())
            {
            actualMembersCount++;
            ASSERT_TRUE(memberVal.IsNull());
            }
        ASSERT_EQ(expectedMembersCount, actualMembersCount);

        IECSqlValue const& structMemberStructArrayVal = stmt.GetValue(1);
        ASSERT_EQ(0, structMemberStructArrayVal.GetArrayLength());

        IECSqlValue const& structArrayVal = stmt.GetValue(2);
        ASSERT_EQ(0, structArrayVal.GetArrayLength());
        }

    //*** array with two null elements
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(L_Array, Struct.l_array, Struct.struct_array, Struct_Array) "
                      "VALUES(?,?,?,?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        for (int i = 1; i <= 4; i++)
            {
            IECSqlBinder& arrayBinder = stmt.GetBinder(i);
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindNull());
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindNull());
            }

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT L_Array, Struct.l_array, Struct.struct_array, Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            IECSqlValue const& val = stmt.GetValue(i);
            ASSERT_FALSE(val.IsNull()) << i << " " << stmt.GetECSql();
            ASSERT_EQ(2, val.GetArrayLength());
            for (IECSqlValue const& elementVal : val.GetArrayIterable())
                {
                ASSERT_TRUE(elementVal.IsNull()) << i << " " << stmt.GetECSql();

                if (val.GetColumnInfo().GetDataType().IsStructArray())
                    {
                    for (IECSqlValue const& memberVal : elementVal.GetStructIterable())
                        {
                        ASSERT_TRUE(memberVal.IsNull());
                        }
                    }
                }
            }
        }

    //*** nested struct array being null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(Struct_Array) VALUES(?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        IECSqlBinder& elementBinder = stmt.GetBinder(1).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["struct_array"].BindNull());
        
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

        ASSERT_FALSE(stmt.IsValueNull(0));
        IECSqlValue const& outerArrayVal = stmt.GetValue(0);
        ASSERT_EQ(1, outerArrayVal.GetArrayLength());
        IECSqlValue const& outerArrayElementVal = *outerArrayVal.GetArrayIterable().begin();
        ASSERT_TRUE(outerArrayElementVal.IsNull()) << stmt.GetECSql();
        int memberCount = 0;
        for (IECSqlValue const& memberVal : outerArrayElementVal.GetStructIterable())
            {
            memberCount++;
            ASSERT_TRUE(memberVal.IsNull());
            }
        ASSERT_EQ((int) outerArrayElementVal.GetColumnInfo().GetStructType()->GetPropertyCount(), memberCount) << "Test class: " << testClassName << " struct: " << outerArrayElementVal.GetColumnInfo().GetStructType()->GetName().c_str();
        }

    //*** nested struct being partially set
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(Struct_Array) VALUES(?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        IECSqlBinder& arrayElementBinder = stmt.GetBinder(1).AddArrayElement();
        //Set StructArray[0].struct_array[0].i=3
        IECSqlBinder& nestedArrayElementBinder = arrayElementBinder["struct_array"].AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, nestedArrayElementBinder["i"].BindInt(3));

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

        ASSERT_FALSE(stmt.IsValueNull(0));
        IECSqlValue const& arrayVal = stmt.GetValue(0);
        ASSERT_EQ(1, arrayVal.GetArrayLength());
        IECSqlValue const& structArrayElementVal = *arrayVal.GetArrayIterable().begin();
        ASSERT_FALSE(structArrayElementVal.IsNull()) << stmt.GetECSql();
        for (IECSqlValue const& memberVal : structArrayElementVal.GetStructIterable())
            {
            if (memberVal.GetColumnInfo().GetProperty()->GetName().Equals("struct_array"))
                {
                ASSERT_EQ(1, memberVal.GetArrayLength()) << "struct_array";
                int memberCount = 0;
                IECSqlValue const& nestedStructVal = *memberVal.GetArrayIterable().begin();
                for (IECSqlValue const& nestedMemberVal : nestedStructVal.GetStructIterable())
                    {
                    memberCount++;
                    if (nestedMemberVal.GetColumnInfo().GetProperty()->GetName().Equals("i"))
                        ASSERT_EQ(3, nestedMemberVal.GetInt());
                    else
                        ASSERT_TRUE(nestedMemberVal.IsNull());
                    }
                ASSERT_EQ((int) nestedStructVal.GetColumnInfo().GetStructType()->GetPropertyCount(), memberCount);
                }
            else
                ASSERT_TRUE(memberVal.IsNull());

            }
        }
    
    // points partially null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(P2D.X, P3D.Y, Struct.pstruct.p2d.X, Struct.pstruct.p3d.Y) VALUES(?,?,?,?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, 3.14)) << ecsql.c_str();

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT P2D, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, Struct.pstruct.p2d.X, Struct.pstruct.p2d.Y, Struct.pstruct.p3d.X, Struct.pstruct.p3d.Y, Struct.pstruct.p3d.Z FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        std::set<Utf8String> notNullItems {"P2D", "P2D.X", "P3D.Y", "Struct.pstruct.p2d.X", "Struct.pstruct.p3d.Y"};
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            IECSqlValue const& val = stmt.GetValue(i);
            Utf8String propPath = val.GetColumnInfo().GetPropertyPath().ToString();
            const bool expectedToBeNull = notNullItems.find(propPath) == notNullItems.end();
            ASSERT_EQ(expectedToBeNull, val.IsNull()) << "Select clause item " << i << " in " << stmt.GetECSql();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IsNullForIncompletePoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IsNullForIncompletePoints.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECProperty propertyName='Code' typeName='string'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element (P2D.X, Code) VALUES (21.5,'C1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P2D, P3D FROM ts.Element WHERE Code='C1'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    IECSqlValue const& point2D = stmt.GetValue(0);
    IECSqlValue const& point3D = stmt.GetValue(1);

    //IsNull only returns true if all coordinates cols are NULL.
    ASSERT_FALSE(point2D.IsNull());
    ASSERT_TRUE(point3D.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                     Krischan.Eberle            02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PrimitiveArrayUnsetMembers)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlprimarray_unsetmembers.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="MyClass">
            <ECArrayProperty propertyName="Long_Array" typeName="long"/>
            <ECArrayProperty propertyName="String_Array" typeName="string"/>
            <ECArrayProperty propertyName="Blob_Array" typeName="binary"/>
            <ECArrayProperty propertyName="P3d_Array" typeName="Point3d"/>
        </ECEntityClass>
    </ECSchema>
    )xml")));

    const uint64_t testValue = BeBriefcaseBasedId(BeBriefcaseId(444), INT64_C(1432342)).GetValue();
    std::vector<std::pair<PrimitiveType, ECValue>> testValues {{PrimitiveType::PRIMITIVETYPE_Long, ECValue(testValue)},
    {PrimitiveType::PRIMITIVETYPE_String, ECValue(R"text(Hello, "world")text")},
    {PrimitiveType::PRIMITIVETYPE_Binary, ECValue((Byte const*) &testValue,sizeof(testValue))},
    {PrimitiveType::PRIMITIVETYPE_Point3d, ECValue(DPoint3d::From(1.0, -1.5, 10.3))}};

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(Long_Array,String_Array,Blob_Array,P3d_Array) VALUES(?,?,?,?)"));

    for (int i = 0; i < 4; i++)
        {
        IECSqlBinder& arrayBinder = stmt.GetBinder(i + 1);
        //first element: don't bind anything
        arrayBinder.AddArrayElement();

        {
        //call BindNull on element
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
        }

        {
        //call BindNull on element
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        std::pair<PrimitiveType, ECValue> const& testValue = testValues[(size_t) i];
        switch (testValue.first)
            {
                case PRIMITIVETYPE_Long:
                    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(testValue.second.GetLong()));
                    break;

                case PRIMITIVETYPE_String:
                    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(testValue.second.GetUtf8CP(), IECSqlBinder::MakeCopy::No));
                    break;

                case PRIMITIVETYPE_Binary:
                {
                size_t blobSize = 0;
                void const* blob = testValue.second.GetBinary(blobSize);
                ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
                break;
                }

                case PRIMITIVETYPE_Point3d:
                    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindPoint3d(testValue.second.GetPoint3d()));
                    break;

                default:
                    FAIL() << "Test has to be adjusted to new primitive type";
            }
        }
        }


    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();

    Statement validateStmt;
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.Prepare(m_ecdb, "SELECT Long_Array,String_Array,Blob_Array,P3d_Array FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());


    for (int i = 0; i < 4; i++)
        {
        Utf8String actualJson(validateStmt.GetValueText(i));
        
        rapidjson::Document expectedJson(rapidjson::kArrayType);
        expectedJson.PushBack(rapidjson::Value(rapidjson::kNullType).Move(), expectedJson.GetAllocator());
        expectedJson.PushBack(rapidjson::Value(rapidjson::kNullType).Move(), expectedJson.GetAllocator());

        std::pair<PrimitiveType, ECValue> const& testValue = testValues[(size_t) i];
        switch (testValue.first)
            {
                case PRIMITIVETYPE_Long:
                    expectedJson.PushBack(rapidjson::Value(testValue.second.GetLong()).Move(), expectedJson.GetAllocator());
                    break;

                case PRIMITIVETYPE_String:
                {
                rapidjson::GenericStringRef<Utf8Char> stringValue(testValue.second.GetUtf8CP(), (rapidjson::SizeType) strlen(testValue.second.GetUtf8CP()));
                expectedJson.PushBack(rapidjson::Value(stringValue).Move(), expectedJson.GetAllocator());
                break;
                }

                case PRIMITIVETYPE_Binary:
                {
                size_t blobSize = 0;
                Byte const* blob = testValue.second.GetBinary(blobSize);

                rapidjson::Value val;
                ASSERT_EQ(SUCCESS, ECRapidJsonUtilities::BinaryToJson(val, blob, blobSize, expectedJson.GetAllocator()));
                expectedJson.PushBack(val.Move(), expectedJson.GetAllocator());
                break;
                }

                case PRIMITIVETYPE_Point3d:
                {
                rapidjson::Value val;
                ASSERT_EQ(SUCCESS, ECRapidJsonUtilities::Point3dToJson(val, testValue.second.GetPoint3d(), expectedJson.GetAllocator()));
                expectedJson.PushBack(val.Move(), expectedJson.GetAllocator());
                break;
                }
                default:
                    FAIL() << "Test has to be adjusted to new primitive type";

            }

        rapidjson::StringBuffer expectedJsonStr;
        rapidjson::Writer<rapidjson::StringBuffer> writer(expectedJsonStr);
        expectedJson.Accept(writer);
        ASSERT_STRCASEEQ(expectedJsonStr.GetString(), actualJson.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsitest                                     Krischan.Eberle            02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayUnsetMembers)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstructarray_unsetmembers.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="MyClass">
                <ECStructArrayProperty propertyName="Locations" typeName="LocationStruct"/>
          </ECEntityClass>
          <ECStructClass typeName="LocationStruct">
                <ECProperty propertyName="Street" typeName="string"/>
                <ECStructProperty propertyName="City" typeName="CityStruct"/>
          </ECStructClass>
         <ECStructClass typeName="CityStruct">
               <ECProperty propertyName="Name" typeName="string"/>
               <ECProperty propertyName="State" typeName="string"/>
               <ECProperty propertyName="Country" typeName="string"/>
               <ECProperty propertyName="Zip" typeName="int"/>
         </ECStructClass>
        </ECSchema>
        )xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(Locations) VALUES(?)"));
    IECSqlBinder& structArrayBinder = stmt.GetBinder(1);
    //first element: don't bind anything
    structArrayBinder.AddArrayElement();

    {
    //call BindNull on element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
    }

    {
    //bind to prim member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["Street"].BindText("mainstreet", IECSqlBinder::MakeCopy::No));
    }

    {
    //bind null to prim member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["Street"].BindNull());
    }

    {
    //call BindNull on struct member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["City"].BindNull());
    }

    {
    //call BindNull on prim and struct member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["Street"].BindNull());
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["City"].BindNull());
    }

    {
    //bind to prim member in struct member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["City"]["Zip"].BindInt(34000));
    }

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();

    Statement validateStmt;
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.Prepare(m_ecdb, "SELECT Locations FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    Utf8String actualJson(validateStmt.GetValueText(0));
    actualJson.ReplaceAll(" ", "");
    ASSERT_STRCASEEQ(R"json([null,null,{"Street":"mainstreet"},{"Street":null},{"City":null},{"Street":null,"City":null},{"City":{"Zip":34000}}])json", actualJson.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, DateTimeCast)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("datetimecast.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    DateTime expectedDateOnly(2017, 2, 22);
    DateTime expectedDtUtc(DateTime::Kind::Utc, 2017, 2, 22, 10, 4, 2);

    ECInstanceKey key;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(DateOnly,DtUtc,S) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, expectedDateOnly)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(2, expectedDtUtc)) << stmt.GetECSql();
    double jd = -1.0;
    ASSERT_EQ(SUCCESS, expectedDateOnly.ToJulianDay(jd));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, jd)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    //casts strip away the date time info as it is not part of the actual value. Same when inserting a DateTime object into a non-DateTime property.
    //when retrieving those values, DateTime::Info is expected to be DateTime::Kind::Unspecified
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DateOnly,DtUtc,CAST(DateOnly AS TEXT), CAST(DtUtc AS TEXT), CAST(DateOnly AS REAL), CAST(DtUtc AS REAL), CAST(DateOnly AS Date), CAST(DtUtc AS TimeStamp), S FROM ecsql.P WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

    ASSERT_EQ(9, stmt.GetColumnCount()) << stmt.GetECSql();

    EXPECT_TRUE(expectedDateOnly.Equals(stmt.GetValueDateTime(0))) << "Select clause item 0: Expected: " << expectedDateOnly.ToString().c_str() << " Actual: " << stmt.GetValueDateTime(0).ToString().c_str();
    EXPECT_TRUE(expectedDtUtc.Equals(stmt.GetValueDateTime(1))) << "Select clause item 1: Expected: " << expectedDtUtc.ToString().c_str() << " Actual: " << stmt.GetValueDateTime(1).ToString().c_str();

    for (int i = 2; i < stmt.GetColumnCount(); i++)
        {
        DateTime actual = stmt.GetValueDateTime(i);
        
        //Original DateTime::Info is always lost with cast or if not persisted in DateTime property
        EXPECT_EQ(DateTime::Component::DateAndTime, actual.GetInfo().GetComponent()) << "Select clause item " << i << ": Expected: " << expectedDateOnly.ToString().c_str() << " Actual: " << actual.ToString().c_str();
        EXPECT_EQ(DateTime::Kind::Unspecified, actual.GetInfo().GetKind()) << "Select clause item " << i << ": Expected: " << expectedDateOnly.ToString().c_str() << " Actual: " << actual.ToString().c_str();

        if (i % 2 == 0)
            EXPECT_TRUE(actual.Equals(DateTime(DateTime::Kind::Unspecified, expectedDateOnly.GetYear(), expectedDateOnly.GetMonth(), expectedDateOnly.GetDay(), 0, 0))) << "Select clause item " << i << " Actual: " << actual.ToString().c_str();
        else
            EXPECT_TRUE(actual.Equals(DateTime(DateTime::Kind::Unspecified, expectedDtUtc.GetYear(), expectedDtUtc.GetMonth(), expectedDtUtc.GetDay(), expectedDtUtc.GetHour(), expectedDtUtc.GetMinute(), expectedDtUtc.GetSecond()))) << "Select clause item " << i << " Actual: " << actual.ToString().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForPoints)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, rowCountPerClass));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(PRIMITIVETYPE_Point3d, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
    ASSERT_TRUE(stmt.IsValueNull(0));
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = true;

    ecsqls["SELECT CAST(NULL AS Double) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = false;

    ecsqls["SELECT CAST(NULL AS Point2D) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = false;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT NULL FROM ecsql.PASpatial"] = true;

    ecsqls["SELECT NULL FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = false;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial"] = true;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT CAST(NULL AS Double) FROM ecsql.PASpatial"] = false;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT CAST(NULL AS Point2D) FROM ecsql.PASpatial"] = false;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            ASSERT_EQ(PRIMITIVETYPE_Point3d, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType()) << ecsql;
            actualRowCount++;
            }
        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForStructs)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, rowCountPerClass));

    ECClassCP structType = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(structType != nullptr);

    auto assertColumn = [] (ECClassCR expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsStruct() && &colVal.GetColumnInfo().GetProperty()->GetAsStructProperty()->GetType() == &expected);
        
        if (checkIsNull)
            ASSERT_TRUE(colVal.IsNull());

        for (IECSqlValue const& memberVal : colVal.GetStructIterable())
            {
            if (checkIsNull)
                ASSERT_TRUE(memberVal.IsNull());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    assertColumn(*structType, stmt.GetValue(0), true);
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS ecsql.[PStruct]) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS Double) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = false;

    ecsqls["SELECT NULL FROM ecsql.P "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = false;

    ecsqls["SELECT PStructProp FROM ecsql.PSA "
        "UNION ALL "
        "SELECT NULL FROM ecsql.P"] = true;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            assertColumn(*structType, stmt.GetValue(0), false);
            actualRowCount++;
            }

        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForPrimArrays)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, rowCountPerClass));


    auto assertColumn = [] (PrimitiveType expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsPrimitiveArray());
        ASSERT_EQ(expected, colVal.GetColumnInfo().GetDataType().GetPrimitiveType());
        if (checkIsNull)
            {
            ASSERT_TRUE(colVal.IsNull());
            ASSERT_EQ(0, colVal.GetArrayLength());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS TIMESTAMP[]) FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    assertColumn(PRIMITIVETYPE_DateTime, stmt.GetValue(0), true);
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS TIMESTAMP[]) FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS Double) FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT CAST(NULL AS TimeStamp) FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT NULL FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT Dt_Array FROM ecsql.PSA "
        "UNION ALL "
        "SELECT NULL FROM ecsql.P"] = true;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            assertColumn(PRIMITIVETYPE_DateTime, stmt.GetValue(0), false);
            actualRowCount++;
            }

        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForStructArrays)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, rowCountPerClass));

    ECClassCP structType = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(structType != nullptr);

    auto assertColumn = [] (ECClassCR expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsStructArray());
        ASSERT_TRUE(&colVal.GetColumnInfo().GetProperty()->GetAsStructArrayProperty()->GetStructElementType() == &expected);
        if (checkIsNull)
            {
            ASSERT_TRUE(colVal.IsNull());
            ASSERT_EQ(0, colVal.GetArrayLength());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS ecsql.PStruct[]) FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    assertColumn(*structType, stmt.GetValue(0), true);
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS ecsql.PStruct[]) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS [ecsql].[PStruct][]) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT NULL FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT PStruct_Array FROM ecsql.PSA "
        "UNION ALL "
        "SELECT NULL FROM ecsql.P"] = true;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            assertColumn(*structType, stmt.GetValue(0), false);
            actualRowCount++;
            }

        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, CoalesceInECSql)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,S) VALUES(22, null)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,S) VALUES(null, 'Foo')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,COALESCE(I,S) FROM ecsql.P"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (stmt.IsValueNull(0))
            ASSERT_STREQ("Foo", stmt.GetValueText(1));
        else
            ASSERT_EQ(22, stmt.GetValueInt(1));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, NestedSelectStatementsTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, UnitPrice FROM ECST.Product WHERE UnitPrice > (SELECT AVG(UnitPrice) From ECST.Product) AND UnitPrice < 500"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "SELECT ProductName From ECST.Product WHERE UnitPrice = ?"));
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.BindDouble(1, stmt.GetValueDouble(1))) << "Binding Double value failed";
    ASSERT_TRUE(selectStmt.Step() == BE_SQLITE_ROW);
    ASSERT_STREQ(stmt.GetValueText(0), selectStmt.GetValueText(0));
    stmt.Finalize();

    //Using GetClassId in Nested Select statement
    ECClassId supplierClassId = m_ecdb.Schemas().GetClassId("ECST", "Supplier", SchemaLookupMode::ByAlias);
    ECClassId customerClassId = m_ecdb.Schemas().GetClassId("ECST", "Customer", SchemaLookupMode::ByAlias);
    ECClassId firstClassId = std::min<ECClassId>(supplierClassId, customerClassId);
    ECClassId secondClassId = std::max<ECClassId>(supplierClassId, customerClassId);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, COUNT(*) FROM (SELECT ECClassId FROM ECST.Supplier UNION ALL SELECT ECClassId FROM ECST.Customer) GROUP BY ECClassId ORDER BY ECClassId"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(firstClassId, stmt.GetValueId<ECClassId>(0));
    ASSERT_EQ(4, stmt.GetValueInt(1));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(secondClassId, stmt.GetValueId<ECClassId>(0));
    ASSERT_EQ(3, stmt.GetValueInt(1));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, PredicateFunctionsInNestedSelectStatement)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    //Using Predicate function in nexted select statement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice IN (SELECT UnitPrice FROM ECST.Product WHERE UnitPrice < (SELECT AVG(UnitPrice) FROM ECST.Product WHERE ProductAvailable))"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(223, (int) stmt.GetValueDouble(0));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    stmt.Finalize();

    //Using NOT operator with predicate function in Nested Select statement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice IN (SELECT UnitPrice FROM ECST.Product WHERE UnitPrice > (SELECT AVG(UnitPrice) FROM ECST.Product WHERE NOT ProductAvailable))"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(619, (int) stmt.GetValueDouble(0));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                             Krischan.Eberle                        12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ParametersInNestedSelectStatement)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("parametersinnestedselect.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, rowCountPerClass));


    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE L=123456789 AND I IN (SELECT I FROM ecsql.P WHERE I=123)"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(rowCountPerClass, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE L=? AND I IN (SELECT I FROM ecsql.P WHERE I=?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 123456789)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 123)) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(rowCountPerClass, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    //Case of Paul Daymond who reported this issue

    ECInstanceKey psaKey1, psaKey2, pKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(L,S) VALUES(314,'Test PSA 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey1)) << stmt.GetECSql();
    stmt.Finalize();

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO ecsql.P(MyPSA.Id,S) VALUES(%s,'Test P')", psaKey1.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(pKey)) << stmt.GetECSql();
    stmt.Finalize();

    ecsql.Sprintf("INSERT INTO ecsql.PSA(L,I,S) VALUES(314,%s,'Test PSA 2')", pKey.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey2)) << stmt.GetECSql();
    stmt.Finalize();

    m_ecdb.SaveChanges();
    }

    {
    Utf8String ecsqlWithoutParams;
    ecsqlWithoutParams.Sprintf("SELECT ECInstanceId FROM ecsql.PSA WHERE L=314 AND I IN (SELECT ECInstanceId FROM ecsql.P WHERE MyPSA.Id=%s)", psaKey1.GetInstanceId().ToString().c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsqlWithoutParams.c_str())) << ecsqlWithoutParams.c_str();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.PSA WHERE L=? AND I IN (SELECT ECInstanceId FROM ecsql.P WHERE MyPSA.Id=?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 314)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, psaKey1.GetInstanceId())) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PSA.ECInstanceId FROM ecsql.PSA, ecsql.P WHERE PSA.L=? AND PSA.I=P.ECInstanceId AND P.MyPSA.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 314)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, psaKey1.GetInstanceId())) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, GroupByClauseTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    Utf8CP expectedProductsNames;
    Utf8String actualProductsNames;
    double expectedSumOfAvgPrices;
    double actualSumOfAvgPrices;
    ECSqlStatement stmt;
    //use of simple GROUP BY clause to find AVG(Price) from the Product table
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product GROUP BY ProductName ORDER BY ProductName"));
    expectedProductsNames = "Binder-Desk-Pen-Pen Set-Pencil-";
    expectedSumOfAvgPrices = 1895.67;
    actualSumOfAvgPrices = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualProductsNames.append(stmt.GetValueText(0));
        actualProductsNames.append("-");
        actualSumOfAvgPrices += stmt.GetValueDouble(1);
        }
    ASSERT_STREQ(expectedProductsNames, actualProductsNames.c_str());
    ASSERT_EQ((int) expectedSumOfAvgPrices, (int) actualSumOfAvgPrices);
    stmt.Finalize();

    //using HAVING clause with GROUP BY clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product GROUP BY ProductName Having AVG(UnitPrice)>300.00 ORDER BY ProductName"));
    expectedProductsNames = "Binder-Pen-Pen Set-";
    actualProductsNames = "";
    expectedSumOfAvgPrices = 1556.62;
    actualSumOfAvgPrices = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualProductsNames.append(stmt.GetValueText(0));
        actualProductsNames.append("-");
        actualSumOfAvgPrices += stmt.GetValueDouble(1);
        }
    ASSERT_STREQ(expectedProductsNames, actualProductsNames.c_str());
    ASSERT_EQ((int) expectedSumOfAvgPrices, (int) actualSumOfAvgPrices);
    stmt.Finalize();

    //combined Use of GROUP BY, HAVING and WHERE Clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice<500 GROUP BY ProductName Having AVG(UnitPrice)>200.00 ORDER BY ProductName"));
    expectedProductsNames = "Binder-Pen Set-";
    actualProductsNames = "";
    expectedSumOfAvgPrices = 666.84;
    actualSumOfAvgPrices = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualProductsNames.append(stmt.GetValueText(0));
        actualProductsNames.append("-");
        actualSumOfAvgPrices += stmt.GetValueDouble(1);
        }
    ASSERT_EQ(expectedProductsNames, actualProductsNames);
    ASSERT_EQ((int) expectedSumOfAvgPrices, (int) actualSumOfAvgPrices);
    stmt.Finalize();

    //GROUP BY Clause with more then one parameters
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice), COUNT(ProductName) FROM ECST.Product GROUP BY ProductName, UnitPrice HAVING COUNT(ProductName)>1"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ("Pen", (Utf8String) stmt.GetValueText(0));
    ASSERT_EQ(539, (int) stmt.GetValueDouble(1));
    ASSERT_EQ(3, stmt.GetValueInt(2));
    ASSERT_FALSE(stmt.Step() != BE_SQLITE_DONE);
    stmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, StructInGroupByClause)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT AVG(Phone) FROM ECST.Customer GROUP BY PersonName"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(1650, statement.GetValueInt(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Country FROM ECST.Customer GROUP BY PersonName"));
    int rowCount = 0;
    while (statement.Step() == DbResult::BE_SQLITE_ROW)
        {
        ASSERT_STREQ("USA", statement.GetValueText(0));
        rowCount++;
        }
    ASSERT_EQ(3, rowCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, VerifyLiteralExpressionAsConstants)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100*5, true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+1+ECClassId, true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000/5, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+2+ECClassId, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000+100*5, true, 'LCD')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductName FROM ECST.Product WHERE UnitPrice=100+2+ECClassId"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Table", statement.GetValueText(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice>ECClassId AND ProductName='Chair'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+ECClassId AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+ECClassId AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice=100+3+ECClassId OR ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, WrapWhereClauseInParams)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' OR Company='ABC'"));
    Utf8String nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE ([Customer].[Country]='USA' OR [Customer].[Company]='ABC')") != nativeSql.npos);
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' AND Company='ABC'"));
    nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE [Customer].[Country]='USA' AND [Customer].[Company]='ABC'") != nativeSql.npos);
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' OR Country='DUBAI' AND ContactTitle='AM'"));
    nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE ([Customer].[Country]='USA' OR [Customer].[Country]='DUBAI' AND [Customer].[ContactTitle]='AM')") != nativeSql.npos);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicDelete_SharedTable)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicDeleteSharedTable.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, true);

    ASSERT_FALSE(m_ecdb.TableExists("nsat_DerivedA"));
    ASSERT_FALSE(m_ecdb.TableExists("nsat_DoubleDerivedA"));
    ASSERT_FALSE(m_ecdb.TableExists("nsat_DoubleDerivedC"));

    //Delete all Instances of the base class, all the structArrays and relationships should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    bvector<Utf8String> tableNames = {"ClassA", "BaseHasDerivedA", "DerivedBHasChildren"};

    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, selectSql.c_str())) << "Prepare failed for " << selectSql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Step failed for " << selectSql.c_str();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << "Table " << tableName.c_str() << " is expected to be empty after DELETE FROM nsat.ClassA";
        stmt.Finalize();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicDelete)
    {
    SchemaItem testSchema(NestedStructArrayTestSchemaHelper::s_testSchemaXml);
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicDeleteTest.ecdb", testSchema));

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, false);

    //Delete all Instances of the base class, all the structArrays should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    bvector<Utf8String> tableNames = {"ClassA" , "DerivedA", "DerivedB", "DoubleDerivedA", "DoubleDerivedB", "DoubleDerivedC"};

    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, selectSql.c_str())) << "Prepare failed for " << selectSql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "step failed for " << selectSql.c_str();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << "Table " << tableName.c_str() << " is expected to be empty after DELETE FROM nsat.ClassA";
        stmt.Finalize();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicDeleteWithSubclassesInMultipleTables)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("PolymorphicDeleteTest.ecdb"));

    ECInstanceId fi1Id;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecdbf.ExternalFileInfo(ECInstanceId, Name, Size, RootFolder, RelativePath) VALUES(2, 'testfile.txt', 123, 1, 'myfolder')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    fi1Id = key.GetInstanceId();
    }

    BeBriefcaseBasedId fi2Id;
    {
    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(L"StartupCompany.json");
    DbEmbeddedFileTable& embeddedFileTable = m_ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    fi2Id = embeddedFileTable.Import(&stat, "embed1", testFilePath.GetNameUtf8().c_str(), "JSON");
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(fi2Id.IsValid());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ecdbf.FileInfo WHERE ECInstanceId=?")) << "cannot delete polymorphically if an existing table is involved";
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Remove("embed1"));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NULL FROM ecdbf.FileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi1Id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi2Id));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
   
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicUpdate)
    {
    SchemaItem testSchema(NestedStructArrayTestSchemaHelper::s_testSchemaXml);
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicUpdateTest.ecdb", testSchema));

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, false);

    //Updates the instances of ClassA
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    m_ecdb.SaveChanges();

    bvector<Utf8String> tableNames = {"ClassA", "DerivedA", "DerivedB", "DoubleDerivedA", "DoubleDerivedB", "DoubleDerivedC"};

    Utf8CP expectedValue = "UpdatedValue";
    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectECSql = "SELECT I,T FROM nsat_";
        selectECSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, selectECSql.c_str()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "Int value don't match for statement " << selectECSql.c_str();
        ASSERT_STREQ(expectedValue, stmt.GetValueText(1)) << "String value don't match for statement " << selectECSql.c_str();
        stmt.Finalize();
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicUpdate_SharedTable)
    {
    // Create and populate a sample project
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicUpdateSharedTable.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, true);

    //Updates the instances of ClassA all the Derived Classes Properties values should also be changed. 
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,I,T FROM nsat.ClassA ORDER BY ECInstanceId"));
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        ASSERT_EQ(2, stmt.GetValueInt(2)) << "The values don't match for instance " << stmt.GetValueInt64(0) << " with class id: " << stmt.GetValueInt64(1);
        ASSERT_STREQ("UpdatedValue", stmt.GetValueText(3)) << "The values don't match for instance " << stmt.GetValueInt64(0) << " with class id: " << stmt.GetValueInt64(1);
        }
    stmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, DeleteWithNestedSelectStatements)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ECST.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(9, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ECST.Product WHERE ProductName IN(SELECT ProductName FROM ECST.Product GROUP BY ProductName HAVING COUNT(ProductName)>2 AND ProductName IN(SELECT ProductName FROM ECST.Product WHERE UnitPrice >500))"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ECST.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, UpdateWithNestedSelectStatments)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ECST.Product SET ProductName='Laptop' WHERE ProductName IN(SELECT ProductName FROM ECST.Product GROUP BY ProductName HAVING COUNT(ProductName)>2 AND ProductName IN(SELECT ProductName FROM ECST.Product WHERE UnitPrice >500))"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ECST.Product WHERE ProductName='Laptop'"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(3, stmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertStructArray)
    {
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlTests.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, perClassRowCount));
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(?, ?)";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt64(1, 1000);
    //add three array elements
    const int count = 3;

    IECSqlBinder& arrayBinder = statement.GetBinder(2);

    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        ECSqlStatus stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                    02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, DeleteStructArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicUpdateTest.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml")));

    auto in = NestedStructArrayTestSchemaHelper::CreateECInstances(m_ecdb, 1, "ClassP");

    int insertCount = 0;
    for (auto inst : in)
        {
        ECInstanceInserter inserter(m_ecdb, inst->GetClass(), nullptr);
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*inst));
        insertCount++;
        }

    ECClassCP classP = m_ecdb.Schemas().GetClass("NestedStructArrayTest", "ClassP");
    ASSERT_TRUE(classP != nullptr);

    ECInstanceDeleter deleter(m_ecdb, *classP, nullptr);

    int deleteCount = 0;
    for (auto& inst : in)
        {
        ASSERT_EQ(BE_SQLITE_OK, deleter.Delete(*inst));
        deleteCount++;
        }

    //Verify Inserted Instance have been deleted.
    bvector<IECInstancePtr> out;
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    ASSERT_FALSE(stmt.Step() == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Int64InStructArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("Int64InStructArrays.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECStructClass typeName='MyStruct' >"
        "        <ECProperty propertyName='I' typeName='int' />"
        "        <ECProperty propertyName='I64' typeName='long' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECStructArrayProperty propertyName='StructArrayProp' typeName='MyStruct' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(StructArrayProp) VALUES(?)"));
    IECSqlBinder& arrayElementBinder = stmt.GetBinder(1).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["I"].BindInt(123456));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["I64"].BindInt64(id.GetValue()));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    Statement rawStmt;
    ASSERT_EQ(BE_SQLITE_OK, rawStmt.Prepare(m_ecdb, "SELECT StructArrayProp FROM ts_Foo WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, rawStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, rawStmt.Step()) << rawStmt.GetSql();
    rapidjson::Document actualStructArrayJson;
    ASSERT_TRUE(!actualStructArrayJson.Parse<0>(rawStmt.GetValueText(0)).HasParseError());

    ASSERT_TRUE(actualStructArrayJson.IsArray());
    ASSERT_EQ(123456, actualStructArrayJson[0]["I"]);
    ASSERT_EQ(id.GetValue(), actualStructArrayJson[0]["I64"]) << "Int64 are expected to not be stringified in the JSON";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoAndSystemProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, MyPSA.Id, MyPSA.RelECClassId, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z FROM ecsql.P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    ASSERT_EQ(9, statement.GetColumnCount());
    for (int i = 0; i < 9; i++)
        {
        ECSqlColumnInfo const& colInfo = statement.GetColumnInfo(i);
        ASSERT_FALSE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsPrimitive());

        if (i < 2)
            {
            ASSERT_STREQ("ClassECSqlSystemProperties", colInfo.GetProperty()->GetClass().GetName().c_str());
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType());
            }
        else if (i < 4)
            {
            ECClassCR navPropMemberClass = colInfo.GetProperty()->GetClass();
            ASSERT_STREQ("NavigationECSqlSystemProperties", navPropMemberClass.GetName().c_str());
            ASSERT_TRUE(navPropMemberClass.IsStructClass());
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType());
            }
        else
            {
            ECClassCR pointMemberClass = colInfo.GetProperty()->GetClass();
            ASSERT_STREQ("PointECSqlSystemProperties", pointMemberClass.GetName().c_str());
            ASSERT_TRUE(pointMemberClass.IsStructClass());
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Double, colInfo.GetDataType().GetPrimitiveType());
            }

        ASSERT_TRUE(Utf8String::IsNullOrEmpty(colInfo.GetRootClassAlias()));
        ASSERT_STREQ("P", colInfo.GetRootClass().GetName().c_str());
        }
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.PSAHasPSA_NN LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    ASSERT_EQ(6, statement.GetColumnCount());
    for (int i = 0; i < 6; i++)
        {
        ECSqlColumnInfo const& colInfo = statement.GetColumnInfo(i);
        ASSERT_FALSE(colInfo.IsGeneratedProperty());
        ASSERT_TRUE(colInfo.GetDataType().IsPrimitive());
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType());
        if (i < 2)
            ASSERT_STREQ("ClassECSqlSystemProperties", colInfo.GetProperty()->GetClass().GetName().c_str());
        else
            ASSERT_STREQ("RelationshipECSqlSystemProperties", colInfo.GetProperty()->GetClass().GetName().c_str());

        ASSERT_TRUE(Utf8String::IsNullOrEmpty(colInfo.GetRootClassAlias()));
        ASSERT_STREQ("PSAHasPSA_NN", colInfo.GetRootClass().GetName().c_str());
        }
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoWithJoin)
    {
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, perClassRowCount));

    Utf8CP ecsql = "SELECT c1.ECInstanceId, c2.ECInstanceId, c1.ECClassId, c2.ECClassId FROM ecsql.PSA c1, ecsql.P c2 LIMIT 1";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << ecsql;

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    auto const& value1 = statement.GetValue(0);
    auto const& columnInfo1 = value1.GetColumnInfo();

    ASSERT_FALSE(value1.IsNull());
    ASSERT_FALSE(columnInfo1.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo1.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c1", columnInfo1.GetRootClassAlias());
    ASSERT_STREQ("PSA", columnInfo1.GetRootClass().GetName().c_str());

    auto const& value2 = statement.GetValue(1);
    auto const& columnInfo2 = value2.GetColumnInfo();

    ASSERT_FALSE(value2.IsNull());
    ASSERT_FALSE(columnInfo2.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo2.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c2", columnInfo2.GetRootClassAlias());
    ASSERT_STREQ("P", columnInfo2.GetRootClass().GetName().c_str());

    auto const& value3 = statement.GetValue(2);
    auto const& columnInfo3 = value3.GetColumnInfo();

    ASSERT_FALSE(value3.IsNull());
    ASSERT_FALSE(columnInfo3.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo3.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c1", columnInfo3.GetRootClassAlias());
    ASSERT_STREQ("PSA", columnInfo3.GetRootClass().GetName().c_str());

    auto const& value4 = statement.GetValue(3);
    auto const& columnInfo4 = value4.GetColumnInfo();

    ASSERT_FALSE(value4.IsNull());
    ASSERT_FALSE(columnInfo4.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo4.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c2", columnInfo4.GetRootClassAlias());
    ASSERT_STREQ("P", columnInfo4.GetRootClass().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoAndNavigationAndPointProp)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT MyPSA, P2D, P3D FROM ecsql.P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    ASSERT_EQ(3, statement.GetColumnCount());
    for (int i = 0; i < 3; i++)
        {
        ECSqlColumnInfo const& colInfo = statement.GetColumnInfo(i);
        ASSERT_FALSE(colInfo.IsGeneratedProperty());
        switch (i)
            {
                case 0:
                    ASSERT_TRUE(colInfo.GetDataType().IsNavigation());
                    break;
                case 1:
                    ASSERT_TRUE(colInfo.GetDataType().IsPrimitive());
                    ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Point2d, colInfo.GetDataType().GetPrimitiveType());
                    break;
                case 2:
                    ASSERT_TRUE(colInfo.GetDataType().IsPrimitive());
                    ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Point3d, colInfo.GetDataType().GetPrimitiveType());
                    break;
            }

        ASSERT_STREQ("P", colInfo.GetProperty()->GetClass().GetName().c_str());
        ASSERT_TRUE(Utf8String::IsNullOrEmpty(colInfo.GetRootClassAlias()));
        ASSERT_STREQ("P", colInfo.GetRootClass().GetName().c_str());
        }

    }



//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InvalidBindArrayCalls)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(I_Array, PStruct_Array) VALUES(?,?)"));

    IECSqlBinder& primArrayBinder = statement.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Error, primArrayBinder.BindInt(1)) << "Cannot call BindXXX before calling AddArrayElement on prim array parameter";
    ASSERT_EQ(ECSqlStatus::Error, primArrayBinder["i"].BindInt(1)) << "Cannot call [] before calling AddArrayElement on prim array parameter";
    IECSqlBinder& primArrayElementBinder = primArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, primArrayElementBinder.BindInt(1));
    ASSERT_EQ(ECSqlStatus::Error, primArrayBinder["i"].BindInt(1)) << "Cannot [] on prim array parameter";

    IECSqlBinder& structArrayBinder = statement.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Error, structArrayBinder.BindInt(1)) << "Cannot call BindXXX before calling AddArrayElement on struct array parameter";
    ASSERT_EQ(ECSqlStatus::Error, structArrayBinder["i"].BindInt(1)) << "Cannot call BindXXX before calling AddArrayElement on struct array parameter";

    IECSqlBinder& structArrayElementBinder = structArrayBinder.AddArrayElement();

    ASSERT_EQ(ECSqlStatus::Error, structArrayElementBinder.BindInt(1)) << "Cannot bind prim to struct";
    ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["i"].BindInt(1));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayUpdate)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 2));

    Utf8CP ecsql = "UPDATE  ONLY ecsql.PSA SET L = ?,  PStruct_Array = ? WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    statement.BindInt(3, 123);
    statement.BindInt64(1, 1000);

    //add three array elements
    const uint32_t arraySize = 3;
    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (int i = 0; i < arraySize; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        ECSqlStatus stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    }

void setProductsValues(StandaloneECInstancePtr instance, int ProductId, Utf8CP ProductName, double price)
    {
    instance->SetValue("ProductId", ECValue(ProductId));
    instance->SetValue("ProductName", ECValue(ProductName));
    instance->SetValue("Price", ECValue(price));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayDelete)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 2));
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt(1, 123);

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECInstanceKey pKey;
    ECInstanceKey psaKey;

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(pKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(psaKey));
    m_ecdb.SaveChanges();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, psaKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(2, pKey.GetInstanceId()));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    ASSERT_EQ(pKey.GetInstanceId().GetValue(), key.GetInstanceId().GetValue());
    m_ecdb.AbandonChanges();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(1, (int) psaKey.GetInstanceId().GetValue())) << "Ids cannot be cast to int without potentially losing information. So BindInt cannot be used for ids";
    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(2, (int) pKey.GetInstanceId().GetValue())) << "Ids cannot be cast to int without potentially losing information. So BindInt cannot be used for ids";
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, psaKey.GetInstanceId().GetValue()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(2, pKey.GetInstanceId().GetValue()));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    ASSERT_EQ(pKey.GetInstanceId().GetValue(), key.GetInstanceId().GetValue());
    m_ecdb.AbandonChanges();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));

    Utf8Char psaIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    psaKey.GetInstanceId().ToString(psaIdStr);
    Utf8Char pIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    pKey.GetInstanceId().ToString(pIdStr);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, psaIdStr, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, pIdStr, IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    ASSERT_EQ(pKey.GetInstanceId().GetValue(), key.GetInstanceId().GetValue());
    m_ecdb.AbandonChanges();
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertNullForECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecinstanceidbindnull.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto assertSequence = [] (ECDbCR ecdb, BeInt64Id expectedSequenceValue)
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Val FROM be_Local WHERE Name='ec_instanceidsequence'");
        ASSERT_TRUE(stmt != nullptr);
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        ASSERT_EQ(expectedSequenceValue.GetValue(), stmt->GetValueUInt64(0));
        };

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(?)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    assertSequence(m_ecdb, key.GetInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    assertSequence(m_ecdb, key.GetInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    assertSequence(m_ecdb, key.GetInstanceId());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindSourceAndTargetECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(1111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(2222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(11111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(22222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(1111111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(2222222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement statement;

    auto stat = statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    {
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, ECInstanceId(UINT64_C(111))));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(2, ECInstanceId(UINT64_C(222))));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(1, 1111)) << "Ids cannot be cast to int without potentially losing information. So BindInt is not supported for ids";
    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(2, 2222)) << "Ids cannot be cast to int without potentially losing information. So BindInt is not supported for ids";
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, 11111LL));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(2, 22222LL));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, "1111111", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "2222222", IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindPrimitiveArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    std::vector<int> expectedIntArray = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<Utf8String> expectedStringArray = {"1", "2", "3", "4", "5", "6", "7", "8"};

    ECInstanceKey ecInstanceKey;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PA (I_Array,S_Array) VALUES(:ia,:sa)"));

    IECSqlBinder& arrayBinderI = statement.GetBinder(1);
    for (int arrayElement : expectedIntArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderI.AddArrayElement().BindInt(arrayElement));
        }

    IECSqlBinder& arrayBinderS = statement.GetBinder(2);
    for (Utf8StringCR arrayElement : expectedStringArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderS.AddArrayElement().BindText(arrayElement.c_str(), IECSqlBinder::MakeCopy::No));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(ecInstanceKey));
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I_Array, S_Array FROM ONLY ecsql.PA WHERE ECInstanceId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);
    statement.BindId(1, ecInstanceKey.GetInstanceId());

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    IECSqlValue const& intArray = statement.GetValue(0);
    size_t expectedIndex = 0;
    for (IECSqlValue const& arrayElement : intArray.GetArrayIterable())
        {
        int actualArrayElement = arrayElement.GetInt();
        ASSERT_EQ(expectedIntArray[expectedIndex], actualArrayElement);
        expectedIndex++;
        }

    IECSqlValue const& stringArray = statement.GetValue(1);
    expectedIndex = 0;
    for (IECSqlValue const& arrayElement : stringArray.GetArrayIterable())
        {
        auto actualArrayElement = arrayElement.GetText();
        ASSERT_STREQ(expectedStringArray[expectedIndex].c_str(), actualArrayElement);
        expectedIndex++;
        }
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Insert_BindDateTimeArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PA (Dt_Array, DtUtc_Array) VALUES(:dt,:dtutc)"));

    std::vector<DateTime> testDates = {DateTime(DateTime::Kind::Utc, 2014, 07, 07, 12, 0),
        DateTime(DateTime::Kind::Unspecified, 2014, 07, 07, 12, 0),
        DateTime(DateTime::Kind::Local, 2014, 07, 07, 12, 0)};


    IECSqlBinder& arrayBinderDt = statement.GetBinder(1);

    for (DateTimeCR testDate : testDates)
        {
        IECSqlBinder& arrayElementBinder = arrayBinderDt.AddArrayElement();

        ECSqlStatus expectedStat = testDate.GetInfo().GetKind() == DateTime::Kind::Local ? ECSqlStatus::Error : ECSqlStatus::Success;
        ASSERT_EQ(expectedStat, arrayElementBinder.BindDateTime(testDate));
        }


    IECSqlBinder& arrayBinderDtUtc = statement.GetBinder(2);

    for (DateTimeCR testDate : testDates)
        {
        IECSqlBinder& arrayElementBinder = arrayBinderDtUtc.AddArrayElement();

        ECSqlStatus expectedStat = testDate.GetInfo().GetKind() == DateTime::Kind::Utc ? ECSqlStatus::Success : ECSqlStatus::Error;
        ASSERT_EQ(expectedStat, arrayElementBinder.BindDateTime(testDate)) << testDate.ToString().c_str();
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindPrimArrayWithOutOfBoundsLength)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.ABounded (Prim_Array_Bounded) VALUES(?)"));

    auto bindArrayValues = [&statement] (int count)
        {
        statement.Reset();
        statement.ClearBindings();

        IECSqlBinder& arrayBinder = statement.GetBinder(1);
        for (int i = 0; i < count; i++)
            {
            if (ECSqlStatus::Success != arrayBinder.AddArrayElement().BindInt(i))
                return BE_SQLITE_ERROR;
            }

        return statement.Step();
        };

    //first: array size to bind. second: Expected to succeed
    const std::vector<std::pair<int, bool>> testArrayCounts = {{ 0, false }, { 2, false }, { 5, true }, { 7, true }, { 10, true },
            { 20, true }}; //Bug in ECObjects: ignores maxoccurs and always interprets it as unbounded.

    for (auto const& testArrayCountItem : testArrayCounts)
        {
        const int testArrayCount = testArrayCountItem.first;
        const bool expectedToSucceed = testArrayCountItem.second;
        const DbResult stepStat = bindArrayValues(testArrayCount);
        if (expectedToSucceed)
            ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Binding array of length " << testArrayCount << " is expected to succceed for array parameter with minOccurs=5 and maxOccurs=10";
        else
            ASSERT_EQ(BE_SQLITE_ERROR, stepStat) << "Binding array of length " << testArrayCount << " is expected to fail for array parameter with minOccurs=5 and maxOccurs=10";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindStructArrayWithOutOfBoundsLength)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.ABounded (PStruct_Array_Bounded) VALUES(?)"));

    auto bindArrayValues = [&statement] (int count)
        {
        statement.Reset();
        statement.ClearBindings();

        IECSqlBinder& arrayBinder = statement.GetBinder(1);
        for (int i = 0; i < count; i++)
            {
            if (ECSqlStatus::Success != arrayBinder.AddArrayElement()["i"].BindInt(i))
                return BE_SQLITE_ERROR;
            }

        return statement.Step();
        };

    //first: array size to bind. second: Expected to succeed
    const std::vector<std::pair<int, bool>> testArrayCounts = {{0, false}, {2, false}, {5, true}, {7, true}, {10, true},
    {20, true}}; //Bug in ECObjects: ignores maxoccurs and always interprets it as unbounded.

    for (auto const& testArrayCountItem : testArrayCounts)
        {
        const int testArrayCount = testArrayCountItem.first;
        const bool expectedToSucceed = testArrayCountItem.second;
        const DbResult stepStat = bindArrayValues(testArrayCount);
        if (expectedToSucceed)
            ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Binding array of length " << testArrayCount << " is expected to succceed for array parameter with minOccurs=5 and maxOccurs=10.";
        else
            ASSERT_EQ(BE_SQLITE_ERROR, stepStat) << "Binding array of length " << testArrayCount << " is expected to fail for array parameter with minOccurs=5 and maxOccurs=10";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithStructBinding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    auto testFunction = [this] (Utf8CP insertECSql, bool bindExpectedToSucceed, int structParameterIndex, Utf8CP structValueJson, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueJson, expectedStructValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(m_ecdb, insertECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of ECSQL '" << insertECSql << "' failed";

        auto& binder = statement.GetBinder(structParameterIndex);

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson(bindStatus, statement, expectedStructValue, binder);
        ASSERT_EQ(bindExpectedToSucceed, bindStatus == SUCCESS);
        if (!bindExpectedToSucceed)
            return;

        ECInstanceKey ecInstanceKey;
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step(ecInstanceKey));

        statement.Finalize();
        stat = statement.Prepare(m_ecdb, verifySelectECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of verification ECSQL '" << verifySelectECSql << "' failed";
        statement.BindId(1, ecInstanceKey.GetInstanceId());

        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        IECSqlValue const& structValue = statement.GetValue(structValueIndex);
        VerifyECSqlValue(statement, expectedStructValue, structValue);
        };

    //**** Test 1 *****
    Utf8CP structValueJson = "{"
        " \"b\" : true,"
        " \"bi\" : null,"
        " \"d\" : 3.1415,"
        " \"dt\" : {"
        "     \"type\" : \"datetime\","
        "     \"datetime\" : \"2014-03-27T13:14:00.000\""
        "     },"
        " \"dtUtc\" : {"
        "     \"type\" : \"datetime\","
        "     \"datetime\" : \"2014-03-27T13:14:00.000Z\""
        "     },"
        " \"i\" : 44444,"
        " \"l\" : 444444444,"
        " \"s\" : \"Hello, world\","
        " \"p2d\" : {"
        "     \"type\" : \"point2d\","
        "     \"x\" : 3.1415,"
        "     \"y\" : 5.5555"
        "     },"
        " \"p3d\" : {"
        "     \"type\" : \"point3d\","
        "     \"x\" : 3.1415,"
        "     \"y\" : 5.5555,"
        "     \"z\" : -6.666"
        "     }"
        "}";

    testFunction("INSERT INTO ecsql.PSA (I, PStructProp) VALUES (?, ?)", true, 2, structValueJson, "SELECT I, PStructProp FROM ecsql.PSA WHERE ECInstanceId = ?", 1);

    //**** Test 2 *****
    structValueJson = "{"
        " \"PStructProp\" : {"
        "       \"b\" : true,"
        "       \"bi\" : null,"
        "       \"d\" : 3.1415,"
        "       \"dt\" : {"
        "           \"type\" : \"datetime\","
        "           \"datetime\" : \"2014-03-27T13:14:00.000\""
        "           },"
        "       \"dtUtc\" : {"
        "           \"type\" : \"datetime\","
        "           \"datetime\" : \"2014-03-27T13:14:00.000Z\""
        "           },"
        "       \"i\" : 44444,"
        "       \"l\" : 444444444,"
        "       \"s\" : \"Hello, world\","
        "       \"p2d\" : {"
        "           \"type\" : \"point2d\","
        "           \"x\" : 3.1415,"
        "           \"y\" : 5.5555"
        "           },"
        "       \"p3d\" : {"
        "           \"type\" : \"point3d\","
        "           \"x\" : 3.1415,"
        "           \"y\" : 5.5555,"
        "           \"z\" : -6.666"
        "           }"
        "       }"
        "}";

    testFunction("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", true, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    //**** Type mismatch tests *****
    //SQLite primitives are compatible to each other (test framework does not allow that yet)
    /*structValueJson = "{\"PStructProp\" : {\"bi\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", true, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"s\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", true, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);
    */
    //ECSQL primitives PointXD and DateTime are only compatible with themselves.
    structValueJson = "{\"PStructProp\" : {\"p2d\" : 3.1415 }}";
    testFunction("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"p3d\" : 3.1415 }}";
    testFunction("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"dt\" : 3.1415 }}";
    testFunction("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"dtUtc\" : 3.1415 }}";
    testFunction("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"dtUtc\" : {"
        "\"type\" : \"datetime\","
        "\"datetime\" : \"2014-03-27T13:14:00.000\"}}}";
    testFunction("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 04/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, UpdateWithStructBinding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    //insert some test instances
    auto insertFunction = [this] (ECInstanceKey& ecInstanceKey, Utf8CP insertECSql, int structParameterIndex, Utf8CP structValueToBindJson)
        {
        Json::Value structValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueToBindJson, structValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(m_ecdb, insertECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of ECSQL '" << insertECSql << "' failed";

        IECSqlBinder& structBinder = statement.GetBinder(structParameterIndex);

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson(bindStatus, statement, structValue, structBinder);

        auto stepStat = statement.Step(ecInstanceKey);
        ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStat) << "Execution of ECSQL '" << insertECSql << "' failed";
        };

    auto testFunction = [this] (Utf8CP updateECSql, int structParameterIndex, Utf8CP structValueJson, int ecInstanceIdParameterIndex, ECInstanceKey ecInstanceKey, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueJson, expectedStructValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(m_ecdb, updateECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of ECSQL '" << updateECSql << "' failed";

        stat = statement.BindId(ecInstanceIdParameterIndex, ecInstanceKey.GetInstanceId());
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Binding ECInstanceId to ECSQL '" << updateECSql << "' failed";

        auto& binder = statement.GetBinder(structParameterIndex);

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson(bindStatus, statement, expectedStructValue, binder);
        auto stepStat = statement.Step();

        ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStat);

        statement.Finalize();
        stat = statement.Prepare(m_ecdb, verifySelectECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of verification ECSQL '" << verifySelectECSql << "' failed";
        statement.BindId(1, ecInstanceKey.GetInstanceId());

        stepStat = statement.Step();
        ASSERT_EQ((int) BE_SQLITE_ROW, (int) stepStat);

        IECSqlValue const& structValue = statement.GetValue(structValueIndex);
        VerifyECSqlValue(statement, expectedStructValue, structValue);
        };

    //Insert test instances
    ECInstanceKey psaECInstanceKey;
    insertFunction(psaECInstanceKey, "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)", 1,
                   "{"
                   " \"b\" : true,"
                   " \"bi\" : null,"
                   " \"d\" : 3.1415,"
                   " \"dt\" : {"
                   "     \"type\" : \"datetime\","
                   "     \"datetime\" : \"2014-03-27T13:14:00.000\""
                   "     },"
                   " \"dtUtc\" : {"
                   "     \"type\" : \"datetime\","
                   "     \"datetime\" : \"2014-03-27T13:14:00.000Z\""
                   "     },"
                   " \"i\" : 44444,"
                   " \"l\" : 444444444,"
                   " \"s\" : \"Hello, world\","
                   " \"p2d\" : {"
                   "     \"type\" : \"point2d\","
                   "     \"x\" : 3.1415,"
                   "     \"y\" : 5.5555"
                   "     },"
                   " \"p3d\" : {"
                   "     \"type\" : \"point3d\","
                   "     \"x\" : 3.1415,"
                   "     \"y\" : 5.5555,"
                   "     \"z\" : -6.666"
                   "     }"
                   "}");

    ECInstanceKey saECInstanceKey;
    insertFunction(saECInstanceKey, "INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", 1,
                   "{"
                   " \"PStructProp\" : {"
                   "       \"b\" : true,"
                   "       \"bi\" : null,"
                   "       \"d\" : 3.1415,"
                   "       \"dt\" : {"
                   "           \"type\" : \"datetime\","
                   "           \"datetime\" : \"2014-03-27T13:14:00.000\""
                   "           },"
                   "       \"dtUtc\" : {"
                   "           \"type\" : \"datetime\","
                   "           \"datetime\" : \"2014-03-27T13:14:00.000Z\""
                   "           },"
                   "       \"i\" : 44444,"
                   "       \"l\" : 444444444,"
                   "       \"s\" : \"Hello, world\","
                   "       \"p2d\" : {"
                   "           \"type\" : \"point2d\","
                   "           \"x\" : 3.1415,"
                   "           \"y\" : 5.5555"
                   "           },"
                   "       \"p3d\" : {"
                   "           \"type\" : \"point3d\","
                   "           \"x\" : 3.1415,"
                   "           \"y\" : 5.5555,"
                   "           \"z\" : -6.666"
                   "           }"
                   "       }"
                   "}");

    //**** Test 1 *****
    Utf8CP newStructValueJson = "{"
        " \"b\" : false,"
        " \"bi\" : null,"
        " \"d\" : -6.283,"
        " \"dt\" : null,"
        " \"dtUtc\" : {"
        "     \"type\" : \"datetime\","
        "     \"datetime\" : \"2014-04-01T14:30:00.000Z\""
        "     },"
        " \"i\" : -10,"
        " \"l\" : -100000000000000,"
        " \"s\" : \"Hello, world, once more\","
        " \"p2d\" : {"
        "     \"type\" : \"point2d\","
        "     \"x\" : -2.5,"
        "     \"y\" : 0.0."
        "     },"
        " \"p3d\" : {"
        "     \"type\" : \"point3d\","
        "     \"x\" : -1.0,"
        "     \"y\" : 1.0,"
        "     \"z\" : 0.0"
        "     }"
        "}";

    testFunction("UPDATE ONLY ecsql.PSA SET PStructProp = ? WHERE ECInstanceId = ?", 1, newStructValueJson, 2, psaECInstanceKey, "SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId = ?", 0);

    //**** Test 2 *****
    newStructValueJson = "{"
        " \"PStructProp\" : {"
        "       \"b\" : false,"
        "       \"bi\" : null,"
        "       \"d\" : -6.55,"
        "       \"dt\" : null,"
        "       \"dtUtc\" : {"
        "           \"type\" : \"datetime\","
        "           \"datetime\" : \"2014-04-01T14:33:00.000Z\""
        "           },"
        "       \"i\" : -10,"
        "       \"l\" : -1000000,"
        "       \"s\" : \"Hello, world, once more\","
        "       \"p2d\" : null,"
        "       \"p3d\" : {"
        "           \"type\" : \"point3d\","
        "           \"x\" : -1.0,"
        "           \"y\" : 1.0,"
        "           \"z\" : 0.0"
        "           }"
        "       }"
        "}";

    testFunction("UPDATE ONLY ecsql.SA SET SAStructProp = ? WHERE ECInstanceId = ?", 1, newStructValueJson, 2, saECInstanceKey, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructsInWhereClause)
    {
    SchemaItem schema("<?xml version='1.0' encoding='utf-8' ?>"
                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                      "    <ECStructClass typeName='Name' >"
                      "        <ECProperty propertyName='First' typeName='string' />"
                      "        <ECProperty propertyName='Last' typeName='string' />"
                      "    </ECStructClass>"
                      "    <ECEntityClass typeName='Person' >"
                      "        <ECStructProperty propertyName='FullName' typeName='Name' />"
                      "    </ECEntityClass>"
                      "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", schema));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Person (FullName.[First], FullName.[Last]) VALUES (?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "John", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "Smith", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "John", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "Myer", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName=?"));
    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName<>?"));
    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetNativeSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName IN (?,?,?)"));

    IECSqlBinder& binder1 = stmt.GetBinder(1);
    binder1["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder1["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    IECSqlBinder& binder2 = stmt.GetBinder(2);
    binder2["First"].BindText("Rich", IECSqlBinder::MakeCopy::No);
    binder2["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    IECSqlBinder& binder3 = stmt.GetBinder(3);
    binder3["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder3["Last"].BindText("Smith", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Person SET FullName.[Last]='Meyer' WHERE FullName=?"));

    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName.[Last]=?"));
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindText(1, "Myer", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    ASSERT_EQ(0, verifyStmt.GetValueInt(0));
    verifyStmt.Reset();
    verifyStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindText(1, "Meyer", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    ASSERT_EQ(1, verifyStmt.GetValueInt(0));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ParameterInSelectClause)
    {
    const auto perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, perClassRowCount));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ?, S FROM ecsql.PSA LIMIT 1"));

    BeBriefcaseBasedId expectedId(BeBriefcaseId(3), 444);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expectedId));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(expectedId.GetValue(), statement.GetValueId<ECInstanceId>(0).GetValueUnchecked());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.IsValueNull(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT -?, S FROM ecsql.PSA LIMIT 1"));

    BeBriefcaseBasedId expectedId(BeBriefcaseId(3), 444);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expectedId));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((-1)*expectedId.GetValue(), statement.GetValueId<ECInstanceId>(0).GetValueUnchecked());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.IsValueNull(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT -? AS MyId, S FROM ecsql.PSA LIMIT 1"));

    int64_t expectedId = -123456LL;
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, expectedId));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((-1)*expectedId, statement.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.IsValueNull(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetParameterIndex)
    {
    const auto perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, perClassRowCount));

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :i AND S = :s AND L = :i * 1000000 + 456789");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    int actualParamIndex = statement.GetParameterIndex("i");
    EXPECT_EQ(1, actualParamIndex);
    statement.BindInt(actualParamIndex, 123);

    actualParamIndex = statement.GetParameterIndex("s");
    EXPECT_EQ(2, actualParamIndex);
    statement.BindText(actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

    actualParamIndex = statement.GetParameterIndex("garbage");
    EXPECT_EQ(-1, actualParamIndex);

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    statement.BindInt(1, 123);

    int actualParamIndex = statement.GetParameterIndex("s");
    EXPECT_EQ(2, actualParamIndex);
    statement.BindText(actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

    actualParamIndex = statement.GetParameterIndex("l");
    EXPECT_EQ(3, actualParamIndex);
    statement.BindInt64(actualParamIndex, 123456789);

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    statement.BindInt(1, 123);

    int actualParamIndex = statement.GetParameterIndex("s");
    EXPECT_EQ(2, actualParamIndex);
    statement.BindText(actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

    statement.BindInt64(3, 123456789);

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :value")) << "VALUE is a reserved word in the ECSQL grammar, so cannot be used without escaping, even in parameter names";
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (L,S,I) VALUES (?,?,:[value])"));

    int actualParamIndex = statement.GetParameterIndex("value");
    ASSERT_EQ(3, actualParamIndex);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(actualParamIndex, 300471));

    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(newKey));

    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.PSA WHERE ECInstanceId = :[id]"));
    actualParamIndex = statement.GetParameterIndex("id");
    ASSERT_EQ(1, actualParamIndex);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(actualParamIndex, newKey.GetInstanceId()));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(newKey.GetInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NoECClassIdFilterOption)
    {
    const auto perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, perClassRowCount));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=True"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=False"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=0"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=1"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ReadonlyPropertiesAreUpdatable)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ReadonlyPropertiesAreUpdatable.ecdb", 
                           SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                               "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                               "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                               "    <ECEntityClass typeName='Element'>"
                               "        <ECCustomAttributes>"
                               "            <ClassMap xmlns='ECDbMap.02.00'>"
                               "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                               "            </ClassMap>"
                               "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                               "        </ECCustomAttributes>"
                               "        <ECProperty propertyName='ReadonlyProp1' typeName='int' readOnly='True' />"
                               "    </ECEntityClass>"
                               "    <ECEntityClass typeName='SubElement'>"
                               "        <BaseClass>Element</BaseClass>"
                               "        <ECProperty propertyName='ReadonlyProp2' typeName='int' readOnly='True' />"
                               "    </ECEntityClass>"
                               "</ECSchema>")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(ReadonlyProp1,ReadonlyProp2) VALUES(1,2)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.SubElement SET ReadonlyProp1=10, ReadonlyProp2=20"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.SubElement SET ReadonlyProp1=10, ReadonlyProp2=20 ECSQLOPTIONS ReadonlyPropertiesAreUpdatable"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //verify update worked
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ReadonlyProp1, ReadonlyProp2 FROM ts.SubElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(10, stmt.GetValueInt(0));
    ASSERT_EQ(20, stmt.GetValueInt(1));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PropertyPathEntry
    {
    Utf8String m_entry;
    bool m_isArrayIndex;

    PropertyPathEntry(Utf8CP entry, bool isArrayIndex) :m_entry(entry), m_isArrayIndex(isArrayIndex) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertColumnInfo(Utf8CP expectedPropertyName, bool expectedIsGenerated, Utf8CP expectedPropPathStr, Utf8CP expectedRootClassName, Utf8CP expectedRootClassAlias, ECSqlColumnInfoCR actualColumnInfo)
    {
    auto actualProperty = actualColumnInfo.GetProperty();
    if (expectedPropertyName == nullptr)
        {
        EXPECT_TRUE(actualProperty == nullptr);
        }
    else
        {
        ASSERT_TRUE(actualProperty != nullptr);
        EXPECT_STREQ(expectedPropertyName, actualProperty->GetName().c_str());
        }

    EXPECT_EQ(expectedIsGenerated, actualColumnInfo.IsGeneratedProperty());

    ECSqlPropertyPath const& actualPropPath = actualColumnInfo.GetPropertyPath();
    Utf8String actualPropPathStr = actualPropPath.ToString();
    EXPECT_STREQ(expectedPropPathStr, actualPropPathStr.c_str());
    LOG.tracev("Property path: %s", actualPropPathStr.c_str());

    bvector<PropertyPathEntry> expectedPropPathEntries;
    bvector<Utf8String> expectedPropPathTokens;
    BeStringUtilities::Split(expectedPropPathStr, ".", expectedPropPathTokens);
    for (Utf8StringCR token : expectedPropPathTokens)
        {
        bvector<Utf8String> arrayTokens;
        BeStringUtilities::Split(token.c_str(), "[]", arrayTokens);

        if (arrayTokens.size() == 1)
            {
            expectedPropPathEntries.push_back(PropertyPathEntry(token.c_str(), false));
            continue;
            }

        ASSERT_EQ(2, arrayTokens.size());
        expectedPropPathEntries.push_back(PropertyPathEntry(arrayTokens[0].c_str(), false));
        expectedPropPathEntries.push_back(PropertyPathEntry(arrayTokens[1].c_str(), true));
        }

    ASSERT_EQ(expectedPropPathEntries.size(), actualPropPath.Size());

    size_t expectedPropPathEntryIx = 0;
    for (ECSqlPropertyPath::EntryCP propPathEntry : actualPropPath)
        {
        PropertyPathEntry const& expectedEntry = expectedPropPathEntries[expectedPropPathEntryIx];
        if (expectedEntry.m_isArrayIndex)
            {
            ASSERT_EQ(ECSqlPropertyPath::Entry::Kind::ArrayIndex, propPathEntry->GetKind());
            ASSERT_EQ(atoi(expectedEntry.m_entry.c_str()), propPathEntry->GetArrayIndex());
            }
        else
            {
            ASSERT_EQ(ECSqlPropertyPath::Entry::Kind::Property, propPathEntry->GetKind());
            ASSERT_STREQ(expectedEntry.m_entry.c_str(), propPathEntry->GetProperty()->GetName().c_str());
            }

        expectedPropPathEntryIx++;
        }

    EXPECT_STREQ(expectedRootClassName, actualColumnInfo.GetRootClass().GetName().c_str());
    if (expectedRootClassAlias == nullptr)
        EXPECT_TRUE(Utf8String::IsNullOrEmpty(actualColumnInfo.GetRootClassAlias()));
    else
        EXPECT_STREQ(expectedRootClassAlias, actualColumnInfo.GetRootClassAlias());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForPrimitiveArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT c.Dt_Array FROM ecsql.PSA c LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("Dt_Array", false, "Dt_Array", "PSA", "c", topLevelColumnInfo);
    auto const& topLevelArrayValue = stmt.GetValue(0);

    //out of bounds test
    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid()) << "ECSqlStatement::GetColumnInfo (-1) is expected to fail";
    ASSERT_FALSE(stmt.GetColumnInfo(2).IsValid()) << "ECSqlStatement::GetColumnInfo is expected to fail with too large index";
    
    //In array level
    int arrayIndex = 0;
    for (IECSqlValue const& arrayElement : topLevelArrayValue.GetArrayIterable())
        {
        auto const& arrayElementColumnInfo = arrayElement.GetColumnInfo();
        Utf8String expectedPropPath;
        expectedPropPath.Sprintf("Dt_Array[%d]", arrayIndex);
        AssertColumnInfo(nullptr, false, expectedPropPath.c_str(), "PSA", "c", arrayElementColumnInfo);

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForStructs)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    ECSqlColumnInfo const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("SAStructProp", false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);

    //out of bounds test
    {
    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid()) << "Index out of range";
    ASSERT_FALSE(stmt.GetColumnInfo(2).IsValid()) << "Index out of range";
    }

    //SAStructProp.PStructProp level
    IECSqlValue const& topLevelStructValue = stmt.GetValue(0);
    AssertColumnInfo("PStructProp", false, "SAStructProp.PStructProp", "SA", nullptr, topLevelStructValue["PStructProp"].GetColumnInfo());

    //SAStructProp.PStructProp.XXX level
    IECSqlValue const& nestedStructValue = topLevelStructValue["PStructProp"];
    AssertColumnInfo("b", false, "SAStructProp.PStructProp.b", "SA", nullptr, nestedStructValue["b"].GetColumnInfo());
    AssertColumnInfo("bi", false, "SAStructProp.PStructProp.bi", "SA", nullptr, nestedStructValue["bi"].GetColumnInfo());
    AssertColumnInfo("p2d", false, "SAStructProp.PStructProp.p2d", "SA", nullptr, nestedStructValue["p2d"].GetColumnInfo());

    //invalid struct members
    ASSERT_FALSE(nestedStructValue[""].GetColumnInfo().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForStructArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("SAStructProp", false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);
    auto const& topLevelStructValue = stmt.GetValue(0);

    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid());

    //SAStructProp.PStruct_Array level
    auto const& pstructArrayValue = topLevelStructValue["PStruct_Array"];
    AssertColumnInfo("PStruct_Array", false, "SAStructProp.PStruct_Array", "SA", nullptr, pstructArrayValue.GetColumnInfo());

    //out of bounds test
    ASSERT_FALSE(topLevelStructValue[""].GetColumnInfo().IsValid()) << "GetValue ("").GetColumnInfo () for struct value";

    //SAStructProp.PStruct_Array[] level
    int arrayIndex = 0;
    Utf8String expectedPropPath;
    for (IECSqlValue const& arrayElement : pstructArrayValue.GetArrayIterable())
        {
        //first struct member
        auto const& arrayElementFirstColumnInfo = arrayElement["b"].GetColumnInfo();
        ASSERT_FALSE(arrayElementFirstColumnInfo.IsValid());

        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].b", arrayIndex);
        AssertColumnInfo("b", false, expectedPropPath.c_str(), "SA", nullptr, arrayElementFirstColumnInfo);

        //second struct member
        auto const& arrayElementSecondColumnInfo = arrayElement["bi"].GetColumnInfo();
        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].bi", arrayIndex);
        AssertColumnInfo("bi", false, expectedPropPath.c_str(), "SA", nullptr, arrayElementSecondColumnInfo);

        ASSERT_FALSE(arrayElement["foo"].GetColumnInfo().IsValid());

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Step)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ecsql.P"));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step() on ECSQL SELECT statement is expected to be allowed.";
    }

    {
    ECSqlStatement statement;
    ECInstanceKey ecInstanceKey;
    auto stepStat = statement.Step(ecInstanceKey);
    ASSERT_EQ(BE_SQLITE_ERROR, stepStat) << "Step(ECInstanceKey&) on ECSQL SELECT statement is expected to not be allowed.";
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.P (I, L) VALUES (100, 10203)"));

    auto stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Step() on ECSQL INSERT statement is expected to be allowed.";

    statement.Reset();
    ECInstanceKey ecInstanceKey;
    stepStat = statement.Step(ecInstanceKey);
    ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Step(ECInstanceKey&) on ECSQL INSERT statement is expected to be allowed.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, MultipleInsertsWithoutReprepare)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (?, ?)"));

    int firstIntVal = 1;
    Utf8CP firstStringVal = "First insert";
    ECSqlStatus stat = statement.BindInt(1, firstIntVal);
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.BindText(2, firstStringVal, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ECInstanceKey firstECInstanceKey;
    auto stepStat = statement.Step(firstECInstanceKey);
    ASSERT_EQ(BE_SQLITE_DONE, stepStat);

    stat = statement.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stat);

    //second insert with same statement

    int secondIntVal = 2;
    Utf8CP secondStringVal = "Second insert";
    stat = statement.BindInt(1, secondIntVal);
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.BindText(2, secondStringVal, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ECInstanceKey secondECInstanceKey;
    stepStat = statement.Step(secondECInstanceKey);
    ASSERT_EQ(BE_SQLITE_DONE, stepStat);

    //check the inserts
    ASSERT_GT(secondECInstanceKey.GetInstanceId().GetValue(), firstECInstanceKey.GetInstanceId().GetValue());

    statement.Finalize();
    stat = statement.Prepare(m_ecdb, "SELECT ECInstanceId, I, S FROM ecsql.PSA WHERE ECInstanceId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    //check first insert
    stat = statement.BindId(1, firstECInstanceKey.GetInstanceId());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    ASSERT_EQ(firstECInstanceKey.GetInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(firstIntVal, statement.GetValueInt(1));
    ASSERT_STREQ(firstStringVal, statement.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    //check second insert
    stat = statement.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stat = statement.BindId(1, secondECInstanceKey.GetInstanceId());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    ASSERT_EQ(secondECInstanceKey.GetInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(secondIntVal, statement.GetValueInt(1));
    ASSERT_STREQ(secondStringVal, statement.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Reset)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    {
    ECSqlStatement stmt;
    auto stat = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P LIMIT 2");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation for a valid ECSQL failed.";
    int expectedRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        expectedRowCount++;
        }

    stat = stmt.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat) << "After ECSqlStatement::Reset";
    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }
    ASSERT_EQ(expectedRowCount, actualRowCount) << "After resetting ECSqlStatement is expected to return same number of returns as after preparation.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Finalize)
    {
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, perClassRowCount));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }

    ASSERT_EQ(perClassRowCount, actualRowCount);
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";

    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }
    ASSERT_EQ(perClassRowCount, actualRowCount);

    //now finalize and do the exact same stuff. In particular this tests that the cursor is reset so that we get all results
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }

    ASSERT_EQ(perClassRowCount, actualRowCount);
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IssueListener)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 10));

    {
    ECDbIssueListener issueListener(m_ecdb);
    ECSqlStatement stmt;
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "new ECSqlStatement";

    auto stat = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P WHERE I = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Prepare.";
    }

    {
    ECDbIssueListener issueListener(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    ECDbIssue lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.IsIssue()) << "After preparing invalid ECSQL.";
    Utf8String actualLastStatusMessage = lastIssue.GetMessage();
    ASSERT_STREQ(actualLastStatusMessage.c_str(), "Invalid ECClass expression 'blablabla'. ECClasses must always be fully qualified in ECSQL: <schema name or prefix>.<class name>");

    stmt.Finalize();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Finalize";

    //now reprepare with valid ECSQL
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Prepare";
    }
   
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetValueWithPartialPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonreaderpartialpoints.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(P2D.X,P3D.Y,PStructProp.p2d.y,PStructProp.p3d.z) VALUES(1.0, 2.0, 3.0, 4.0)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT P2D,P3D,PStructProp.p2d,PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
    
    //The coordinates of partial points being NULL in the ECDb file will be returned as 0 as this is what SQLite
    //implicitly returns when calling GetValueDouble on a NULL column.
    ASSERT_TRUE(DPoint2d::From(1.0, 0.0).AlmostEqual(selStmt.GetValuePoint2d(0))) << selStmt.GetECSql();
    ASSERT_TRUE(DPoint3d::From(0.0, 2.0, 0.0).AlmostEqual(selStmt.GetValuePoint3d(1))) << selStmt.GetECSql();
    ASSERT_TRUE(DPoint2d::From(0.0, 3.0).AlmostEqual(selStmt.GetValuePoint2d(2))) << selStmt.GetECSql();
    ASSERT_TRUE(DPoint3d::From(0.0, 0.0, 4.0).AlmostEqual(selStmt.GetValuePoint3d(3))) << selStmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertGeometry(IGeometryCR expected, IGeometryCR actual, Utf8CP assertMessage)
    {
    ASSERT_TRUE(actual.IsSameStructureAndGeometry(expected)) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Geometry)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    std::vector<IGeometryPtr> expectedGeoms;

    IGeometryPtr line1 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr line2 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 1.0, 1.0, 2.0, 2.0, 2.0)));
    IGeometryPtr line3 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(2.0, 2.0, 2.0, 3.0, 3.0, 3.0)));

    expectedGeoms.push_back(line1);
    expectedGeoms.push_back(line2);
    expectedGeoms.push_back(line3);
    IGeometryPtr expectedGeomSingle = expectedGeoms[0];

    // insert geometries in various variations
    {
    auto ecsql = "INSERT INTO ecsql.PASpatial (Geometry, B, Geometry_Array) VALUES(?, True, ?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));

    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    {
    auto ecsql = "INSERT INTO ecsql.SSpatial (SpatialStructProp.Geometry, SpatialStructProp.Geometry_Array) VALUES(?,?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));

    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    auto ecsql = "INSERT INTO ecsql.SSpatial (SpatialStructProp) VALUES(?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    IECSqlBinder& structBinder = statement.GetBinder(1);

    ASSERT_EQ(ECSqlStatus::Success, structBinder["Geometry"].BindGeometry(*expectedGeomSingle));

    IECSqlBinder& arrayBinder = structBinder["Geometry_Array"];
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    m_ecdb.SaveChanges();

    //now verify the inserts
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT B, Geometry_Array, Geometry FROM ecsql.PASpatial")) << "Preparation failed";
    int rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ASSERT_TRUE(statement.GetValueBoolean(0)) << "First column value is expected to be true";

        IECSqlValue const& arrayVal = statement.GetValue(1);
        int i = 0;
        for (IECSqlValue const& arrayElem : arrayVal.GetArrayIterable())
            {
            IGeometryPtr actualGeom = arrayElem.GetGeometry();
            ASSERT_TRUE(actualGeom != nullptr);

            AssertGeometry(*expectedGeoms[i], *actualGeom, "PASpatial.Geometry_Array");
            i++;
            }
        ASSERT_EQ((int) expectedGeoms.size(), i);

        IGeometryPtr actualGeom = statement.GetValueGeometry(2);
        ASSERT_TRUE(actualGeom != nullptr);
        AssertGeometry(*expectedGeomSingle, *actualGeom, "PASpatial.Geometry");
        }
    ASSERT_EQ(1, rowCount);

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT SpatialStructProp.Geometry_Array, SpatialStructProp.Geometry FROM ecsql.SSpatial")) << "Preparation failed";
    rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        IECSqlValue const& arrayVal = statement.GetValue(0);
        int i = 0;
        for (IECSqlValue const& arrayElem : arrayVal.GetArrayIterable())
            {
            IGeometryPtr actualGeom = arrayElem.GetGeometry();
            ASSERT_TRUE(actualGeom != nullptr);

            AssertGeometry(*expectedGeoms[i], *actualGeom, "SSpatial.SpatialStructProp.Geometry_Array");
            i++;
            }
        ASSERT_EQ((int) expectedGeoms.size(), i);

        IGeometryPtr actualGeom = statement.GetValueGeometry(1);
        ASSERT_TRUE(actualGeom != nullptr);
        AssertGeometry(*expectedGeomSingle, *actualGeom, "SSpatial.SpatialStructProp.Geometry");
        }
    ASSERT_EQ(2, rowCount);

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT SpatialStructProp FROM ecsql.SSpatial")) << "Preparation failed";
    rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        IECSqlValue const& structVal = statement.GetValue(0);
        for (IECSqlValue const& structMemberVal : structVal.GetStructIterable())
            {
            Utf8StringCR structMemberName = structMemberVal.GetColumnInfo().GetProperty()->GetName();
            if (structMemberName.Equals("Geometry"))
                {
                IGeometryPtr actualGeom = structMemberVal.GetGeometry();
                AssertGeometry(*expectedGeomSingle, *actualGeom, "SSpatial.SpatialStructProp > Geometry");
                }
            else if (structMemberName.Equals("Geometry_Array"))
                {
                int i = 0;
                for (IECSqlValue const& arrayElem : structMemberVal.GetArrayIterable())
                    {
                    IGeometryPtr actualGeom = arrayElem.GetGeometry();
                    ASSERT_TRUE(actualGeom != nullptr);

                    AssertGeometry(*expectedGeoms[i], *actualGeom, "SSpatial.SpatialStructProp > Geometry_Array");
                    i++;
                    }
                ASSERT_EQ((int) expectedGeoms.size(), i);
                }
            }
        }
    ASSERT_EQ(2, rowCount);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GeometryAndOverflow)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsql_geometry.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECArrayProperty propertyName="GeomArray" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Geom_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECArrayProperty propertyName="GeomArray_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml")));

    std::vector<IGeometryPtr> expectedGeoms {IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0))),
        IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 1.0, 1.0, 2.0, 2.0, 2.0))),
        IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(2.0, 2.0, 2.0, 3.0, 3.0, 3.0)))};

    IGeometryPtr expectedGeomSingle = expectedGeoms[0];

    ECInstanceKey key;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Element(Geom,GeomArray,Geom_Overflow,GeomArray_Overflow) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));
    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (IGeometryPtr& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(3, *expectedGeomSingle));
    IECSqlBinder& arrayBinder2 = statement.GetBinder(4);
    for (IGeometryPtr& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindGeometry(*geom));

        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key)) << statement.GetECSql() << " " << statement.GetNativeSql();
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Geom,GeomArray,Geom_Overflow,GeomArray_Overflow FROM ts.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << statement.GetECSql();

    AssertGeometry(*expectedGeomSingle, *statement.GetValueGeometry(0), "Geometry property");
    size_t arrayIndex = 0;
    for (IECSqlValue const& arrayElementVal : statement.GetValue(1).GetArrayIterable())
        {
        AssertGeometry(*expectedGeoms[arrayIndex], *arrayElementVal.GetGeometry(), "Geometry array property");
        arrayIndex++;
        }
    AssertGeometry(*expectedGeomSingle, *statement.GetValueGeometry(2), "Geometry property overflow");
    arrayIndex = 0;
    for (IECSqlValue const& arrayElementVal : statement.GetValue(3).GetArrayIterable())
        {
        AssertGeometry(*expectedGeoms[arrayIndex], *arrayElementVal.GetGeometry(), "Geometry array property overflow");
        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetGeometryWithInvalidBlobFormat)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    // insert invalid geom blob
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "INSERT INTO ecsqltest_PASpatial(Id, Geometry) VALUES (1,?)"));
    double dummyValue = 3.141516;
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindBlob(1, &dummyValue, (int) sizeof(dummyValue), Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "SELECT Geometry FROM ecsql.PASpatial"));
    int rowCount = 0;
    while (ecsqlStmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_FALSE(ecsqlStmt.IsValueNull(0)) << "Geometry column is not expected to be null.";

        ASSERT_TRUE(ecsqlStmt.GetValueGeometry(0) == nullptr) << "Invalid geom blob format expected so that nullptr is returned.";
        }

    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ClassWithStructHavingStructArrayInsert)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.SA (SAStructProp) VALUES(?)"));

    IECSqlBinder& saStructBinder = statement.GetBinder(1); //SAStructProp
    ASSERT_EQ(ECSqlStatus::Success, saStructBinder["PStructProp"]["i"].BindInt(99));

    //add three array elements
    const int count = 3;
    IECSqlBinder& arrayBinder = saStructBinder["PStruct_Array"];
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["d"].BindDouble(i * PI));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["i"].BindInt(i * 2));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["l"].BindInt64(i * 3));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1)));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2)));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << statement.GetECSql();
    statement.Finalize();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto const& pStructArray = stmt.GetValue(0);
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayInsertWithParametersLongAndArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(123, ?)";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    //add three array elements
    const int count = 3;
    IECSqlBinder& arrayBinder = statement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        auto stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "L");
        ASSERT_EQ(123, v.GetLong());
        inst->GetValue(v, "PStruct_Array");
        ASSERT_EQ(count, v.GetArrayInfo().GetCount());
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithMixParametersIntAndInt)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.Sub1 (I,Sub1I) VALUES(123, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt(1, 333);

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "I");
        ASSERT_EQ(123, v.GetInteger());
        inst->GetValue(v, "Sub1I");
        ASSERT_EQ(123, v.GetInteger());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithMixParameters)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.P (B,D,I,L,S) VALUES(1, ?,?,123,?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindDouble(1, 2.22);
    statement.BindInt(2, 123);
    statement.BindText(3, "Test Test", IECSqlBinder::MakeCopy::Yes);
    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "B");
        ASSERT_EQ(true, v.GetBoolean());
        inst->GetValue(v, "D");
        ASSERT_EQ(2.22, v.GetDouble());
        inst->GetValue(v, "I");
        ASSERT_EQ(123, v.GetInteger());
        inst->GetValue(v, "L");
        ASSERT_EQ(123, v.GetLong());
        inst->GetValue(v, "S");
        ASSERT_STREQ("Test Test", v.GetUtf8CP());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ClassWithStructHavingStructArrayInsertWithDotOperator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto &pStructArray = stmt.GetValue(0);
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        //need to Verify all values
        /*  for (auto const & arrayItem : pStructArray)
        {
        IECSqlValue const & value= arrayItem->operator[](0);
        value.GetDouble();
        }

        // ASSERT_TRUE(ECObjectsStatus::Success == inst->GetValue(v, L"SAStructProp.PStruct_Array",0));
        // IECInstancePtr structInstance = v.GetStruct();
        // structInstance->GetValue(v, L"PStruct_Array");
        //ASSERT_TRUE(v.IsArray());
        ASSERT_TRUE(pStructArray == pStructArray);*/
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, StructUpdateWithDotOperator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp.i) VALUES(2)";

    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    {
    auto prepareStatus = statement.Prepare(m_ecdb, "SELECT * FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(statement);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "SAStructProp.PStructProp.i");
        ASSERT_EQ(2, v.GetInteger());
        }
    }
    statement.Finalize();
    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp.i = 3 ";
    stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();

    auto prepareStatus = statement.Prepare(m_ecdb, "SELECT * FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(statement);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "SAStructProp.PStructProp.i");
        ASSERT_EQ(3, v.GetInteger());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ClassWithStructHavingStructArrayUpdateWithDotOperator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement insertStatement;
    auto stat = insertStatement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    //add three array elements
    int count = 3;
    IECSqlBinder& arrayBinder = insertStatement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = insertStatement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    ECSqlStatement selectStatement;
    auto prepareStatus = selectStatement.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (selectStatement.Step() == BE_SQLITE_ROW)
        {
        auto const& pStructArray = selectStatement.GetValue(0);
        ASSERT_EQ(count, pStructArray.GetArrayLength());
        }

    ECSqlStatement updateStatement;
    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStruct_Array=?";
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.Prepare(m_ecdb, ecsql)) << ecsql;
    count = 3;
    IECSqlBinder& updateArrayBinder = updateStatement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = updateArrayBinder.AddArrayElement();
        stat = arrayElementBinder["d"].BindDouble(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    stepStatus = updateStatement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";

    ECSqlStatement statement;
    prepareStatus = statement.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto const& pStructArray = statement.GetValue(0);
        ASSERT_EQ(count, pStructArray.GetArrayLength());
        }
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, AmbiguousQuery)
    {
    SchemaItem schemaXml("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                         "    <ECStructClass typeName='Struct' >"
                         "        <ECProperty propertyName='P1' typeName='string' />"
                         "        <ECProperty propertyName='P2' typeName='int' />"
                         "    </ECStructClass>"
                         "    <ECEntityClass typeName='TestClass' >"
                         "        <ECProperty propertyName='P1' typeName='string'/>"
                         "         <ECStructProperty propertyName = 'TestClass' typeName = 'Struct'/>"
                         "    </ECEntityClass>"
                         "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("AmbiguousQuery.ecdb", schemaXml));

    ECN::ECSchemaCP schema = m_ecdb.Schemas().GetSchema("TestSchema");

    ECClassCP TestClass = schema->GetClassCP("TestClass");
    ASSERT_TRUE(TestClass != nullptr);

    ECN::StandaloneECInstancePtr Instance1 = TestClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr Instance2 = TestClass->GetDefaultStandaloneEnabler()->CreateInstance();

    Instance1->SetValue("P1", ECValue("Harvey"));
    Instance1->SetValue("TestClass.P1", ECValue("val1"));
    Instance1->SetValue("TestClass.P2", ECValue(123));

    Instance2->SetValue("P1", ECValue("Mike"));
    Instance2->SetValue("TestClass.P1", ECValue("val2"));
    Instance2->SetValue("TestClass.P2", ECValue(345));

    //Inserting values of TestClass
    ECInstanceInserter inserter(m_ecdb, *TestClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*Instance1, true));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*Instance2, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1 FROM ts.TestClass"));
    Utf8String ExpectedValueOfP1 = "Harvey-Mike-";
    Utf8String ActualValueOfP1;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ActualValueOfP1.append(stmt.GetValueText(0));
        ActualValueOfP1.append("-");
        }

    ASSERT_EQ(ExpectedValueOfP1, ActualValueOfP1);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TestClass.TestClass.P1 FROM ts.TestClass"));
    Utf8String ExpectedValueOfStructP1 = "val1-val2-";
    Utf8String ActualValueOfStructP1;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ActualValueOfStructP1.append(stmt.GetValueText(0));
        ActualValueOfStructP1.append("-");
        }

    ASSERT_EQ(ExpectedValueOfStructP1, ActualValueOfStructP1);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TestClass.P2 FROM ts.TestClass"));
    int ActualValueOfStructP2 = 468;
    int ExpectedValueOfStructP2 = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ExpectedValueOfStructP2 += stmt.GetValueInt(0);
        }

    ASSERT_EQ(ExpectedValueOfStructP2, ActualValueOfStructP2);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                 10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindNegECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindNegECInstanceId.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    ECSqlStatement stmt;

    //Inserting Values for a negative ECInstanceId.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P (ECInstanceId,B,D,S) VALUES(?,?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, (int64_t) (-1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(2, true));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 100.54));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, "Foo", IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Retrieving Values.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "Select B,D,S FROM ecsql.P WHERE ECInstanceId=-1"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(true, stmt.GetValueBoolean(0));
    ASSERT_EQ(100.54, stmt.GetValueDouble(1));
    ASSERT_STREQ("Foo", stmt.GetValueText(2));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InstanceInsertionInArray)
    {
    SchemaItem schema("<?xml version='1.0' encoding='utf-8'?>"
                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                      "    <ECEntityClass typeName='TestClass' >"
                      "        <ECProperty propertyName='P1' typeName='string'/>"
                      "        <ECArrayProperty propertyName = 'P2_Array' typeName = 'string' minOccurs='1' maxOccurs='2'/>"
                      "        <ECArrayProperty propertyName = 'P3_Array' typeName = 'int' minOccurs='1' maxOccurs='2'/>"
                      "    </ECEntityClass>"
                      "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("InstanceInsertionInArray.ecdb", schema));

    std::vector<Utf8String> expectedStringArray = {"val1", "val2", "val3"};
    std::vector<int> expectedIntArray = {200, 400, 600};

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (P1, P2_Array, P3_Array) VALUES(?, ?, ?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo", IECSqlBinder::MakeCopy::No));

    IECSqlBinder& arrayBinderS = stmt.GetBinder(2);
    for (Utf8StringCR arrayElement : expectedStringArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderS.AddArrayElement().BindText(arrayElement.c_str(), IECSqlBinder::MakeCopy::No));
        }

    IECSqlBinder& arrayBinderI = stmt.GetBinder(3);
    for (int arrayElement : expectedIntArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderI.AddArrayElement().BindInt(arrayElement));
        }
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2_Array, P3_Array FROM ts.TestClass"));
    while (stmt.Step() == DbResult::BE_SQLITE_ROW)
        {
        ASSERT_STREQ("Foo", stmt.GetValueText(0));

        IECSqlValue const& StringArray = stmt.GetValue(1);
        size_t expectedIndex = 0;

        for (IECSqlValue const& arrayElement : StringArray.GetArrayIterable())
            {
            Utf8CP actualArrayElement = arrayElement.GetText();
            ASSERT_STREQ(expectedStringArray[expectedIndex].c_str(), actualArrayElement);
            expectedIndex++;
            }

        IECSqlValue const& IntArray = stmt.GetValue(2);
        expectedIndex = 0;
        for (IECSqlValue const& arrayElement : IntArray.GetArrayIterable())
            {
            int actualArrayElement = arrayElement.GetInt();
            ASSERT_EQ(expectedIntArray[expectedIndex], actualArrayElement);
            expectedIndex++;
            }
        }
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SelectAfterImport)
    {
    auto importSchema = [] (ECDbCR db, ECN::ECSchemaCR schemaIn)
        {
        ECN::ECSchemaCP existing = db.Schemas().GetSchema(schemaIn.GetName().c_str());
        if (nullptr != existing)
            return;

        ECN::ECSchemaPtr imported = nullptr;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, schemaIn.CopySchema(imported));

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetName(schemaIn.GetName()));

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetAlias(schemaIn.GetAlias()));

        ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
        ASSERT_EQ(ECN::ECObjectsStatus::Success, contextPtr->AddSchema(*imported));

        ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(contextPtr->GetCache().GetSchemas()));
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ImportTwoInARow.ecdb"));

    {
    ECN::ECSchemaPtr schema;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ECN::ECSchema::CreateSchema(schema, "ImportTwoInARow", "tir", 0, 0, 0));

    ECN::ECEntityClassP ecclass;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, schema->CreateEntityClass(ecclass, "C1"));

    ECN::PrimitiveECPropertyP ecprop;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecclass->CreatePrimitiveProperty(ecprop, "X"));

    ecprop->SetType(ECN::PRIMITIVETYPE_Double);

    importSchema(m_ecdb, *schema);
    }

    EC::ECSqlStatement selectC1;
    selectC1.Prepare(m_ecdb, "SELECT ECInstanceId FROM tir.C1");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   09/12
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, SelectCountPolymorphic)
    {
    auto getCount = [](ECDbCR db, ECClassCR ecClass, bool isPolymorphic)
        {
        Utf8String ecsql("SELECT count(*) FROM ");

        if (!isPolymorphic)
            ecsql.append("ONLY ");

        ecsql.append(ecClass.GetECSqlName());

        ECSqlStatement statement;
        ECSqlStatus stat = statement.Prepare(db, ecsql.c_str());
        if (stat != ECSqlStatus::Success)
            return -1;

        if (statement.Step() != BE_SQLITE_ROW)
            return -1;

        return statement.GetValueInt(0);
        };

    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));

    /*
    * Test retrieval when parent and children are all in the same table (TablePerHierarchy)
    */
    ECClassCP furniture = m_ecdb.Schemas().GetClass("StartupCompany", "Furniture");
    ASSERT_TRUE(furniture != nullptr);
    int count = getCount(m_ecdb, *furniture, false);
    ASSERT_EQ(3, count);
    count = getCount(m_ecdb, *furniture, true);
    ASSERT_EQ(9, count);

    /*
    * Test retrieval when parent and children are all in different tables (TablePerClass)
    */
    ECClassCP hardware = m_ecdb.Schemas().GetClass("StartupCompany", "Hardware");
    ASSERT_TRUE(hardware != nullptr);
    count = getCount(m_ecdb, *hardware, false);
    ASSERT_EQ(3, count);
    count = getCount(m_ecdb, *hardware, true);
    ASSERT_EQ(9, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                           07/12
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, SelectClause)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(m_ecdb, 3));
    ECClassCP employee = m_ecdb.Schemas().GetClass("StartupCompany", "Employee");
    ASSERT_TRUE(employee != nullptr);

    ECSqlStatement ecStatement;

    Utf8String jobTitle1;
    int managerId1;
    {
    // ECSQL should honor the order of the ecColumns from the select clause
    ASSERT_EQ(ECSqlStatus::Success, ecStatement.Prepare(m_ecdb, "SELECT JobTitle, ManagerID FROM [StartupCompany].[Employee]"));
    ASSERT_EQ(BE_SQLITE_ROW, ecStatement.Step());
    jobTitle1 = ecStatement.GetValueText(0);
    managerId1 = ecStatement.GetValueInt(1);
    EXPECT_TRUE(ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("JobTitle"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ManagerID"));
    ecStatement.Finalize();
    }

    {
    ASSERT_EQ(ECSqlStatus::Success, ecStatement.Prepare(m_ecdb, "SELECT JobTitle, ManagerID FROM [StartupCompany].[Employee]"));
    ASSERT_EQ(BE_SQLITE_ROW, ecStatement.Step());
    Utf8String jobTitle2 = ecStatement.GetValueText(0);
    int        managerId2 = ecStatement.GetValueInt(1);
    EXPECT_TRUE(ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("JobTitle"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ManagerID"));
    ecStatement.Finalize();

    ASSERT_EQ(managerId1, managerId2);
    ASSERT_TRUE(jobTitle1.Equals(jobTitle2));
    }

    {
    // ECSQL SelectAll (aka '*') should select in same order as ECProperties appear in the ECSchema
    ASSERT_TRUE(ECSqlStatus::Success == ecStatement.Prepare(m_ecdb, "SELECT * FROM [StartupCompany].[Employee]"));
    ASSERT_TRUE(BE_SQLITE_ROW == ecStatement.Step());
    EXPECT_TRUE(ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("ECInstanceId"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ECClassId"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(2).GetProperty()->GetName().Equals("EmployeeID"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(3).GetProperty()->GetName().Equals("FirstName"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(4).GetProperty()->GetName().Equals("JobTitle"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(5).GetProperty()->GetName().Equals("LastName"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(6).GetProperty()->GetName().Equals("ManagerID"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(7).GetProperty()->GetName().Equals("Room"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(8).GetProperty()->GetName().Equals("SSN"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(9).GetProperty()->GetName().Equals("Project"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(10).GetProperty()->GetName().Equals("WorkPhone"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(11).GetProperty()->GetName().Equals("MobilePhone"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(12).GetProperty()->GetName().Equals("FullName"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(13).GetProperty()->GetName().Equals("Certifications"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(14).GetProperty()->GetName().Equals("Location"));
    EXPECT_TRUE(ecStatement.GetValue(14)["Coordinate"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Coordinate"));
    EXPECT_TRUE(ecStatement.GetValue(14)["Street"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Street"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(15).GetProperty()->GetName().Equals("EmployeeType"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(16).GetProperty()->GetName().Equals("Address"));
    EXPECT_TRUE(ecStatement.GetValue(16)["Coordinate"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Coordinate"));
    EXPECT_TRUE(ecStatement.GetValue(16)["Street"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Street"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(17).GetProperty()->GetName().Equals("EmployeeRecordKey"));
    }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   09/12
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, OrderBy)
    {
    // Create StartupCompany 
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));

    auto insertPerson = [](ECDbCR ecdb, ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName)
        {
        IECInstancePtr ecInstance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
        ECValue val;
        val.SetUtf8CP(firstName, false);
        if (ECObjectsStatus::Success != ecInstance->SetValue("FirstName", val))
            return BE_SQLITE_ERROR;

        val.Clear();
        val.SetUtf8CP(lastName, false);
        if (ECObjectsStatus::Success != ecInstance->SetValue("LastName", val))
            return BE_SQLITE_ERROR;

        ECInstanceInserter inserter(ecdb, ecClass, nullptr);
        if (!inserter.IsValid())
            return BE_SQLITE_ERROR;

        return inserter.Insert(*ecInstance);
        };

    // Add some employees
    ECClassCP employeeClass = m_ecdb.Schemas().GetClass("StartupCompany", "Employee");
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Leonardo", "Da Vinci"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Galileo", "Galilei"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Nikola", "Tesla"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Niels", "Bohr"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Albert", "Einstein"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Albert", "Einstein"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Srinivasa", "Ramanujan"));
    m_ecdb.SaveChanges();

    // Retrieve them in alphabetical order
    Utf8String ecsql("SELECT FirstName,LastName FROM ");
    ecsql.append(employeeClass->GetECSqlName()).append(" ORDER BY LastName, FirstName");
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(m_ecdb, ecsql.c_str());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    // Just log for a manual check
    Utf8CP firstName, lastName;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        firstName = statement.GetValueText(0);
        lastName = statement.GetValueText(1);
        LOG.debugv("%s, %s", lastName, firstName);
        }

    // Validate the first few entries
    statement.Reset();
    DbResult stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    lastName = statement.GetValueText(1);
    ASSERT_EQ(0, strcmp(lastName, "Bohr"));

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    lastName = statement.GetValueText(1);
    ASSERT_EQ(0, strcmp(lastName, "Da Vinci"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   09/12
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, LimitOffset)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));

    // Populate 100 instances
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("StartupCompany", "ClassWithPrimitiveProperties");
    IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECInstanceInserter inserter(m_ecdb, *ecClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    for (int ii = 0; ii < 100; ii++)
        {
        ECValue ecValue(ii);
        ASSERT_EQ(ECObjectsStatus::Success, ecInstance->SetValue("intProp", ecValue));
        ECInstanceKey instanceKey;
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(instanceKey, *ecInstance));
        }
    m_ecdb.SaveChanges();

    // Setup query for a page of instances
    Utf8String ecsql("SELECT intProp FROM ");
    ecsql.append(ecClass->GetECSqlName()).append(" LIMIT :pageSize OFFSET :pageSize * (:pageNumber - 1)");
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    int pageSizeIndex = statement.GetParameterIndex("pageSize");
    ASSERT_TRUE(pageSizeIndex >= 0);
    int pageNumberIndex = statement.GetParameterIndex("pageNumber");
    ASSERT_TRUE(pageNumberIndex >= 0);

    // Start getting the 5th page, where each page is 10 instances
    int pageSize = 10;
    int pageNumber = 5;
    statement.BindInt(pageSizeIndex, pageSize);
    statement.BindInt(pageNumberIndex, pageNumber);

    // Verify the first result
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    int actualValue = statement.GetValueInt(0);
    int expectedValue = pageSize * (pageNumber - 1);
    ASSERT_EQ(actualValue, expectedValue);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   09/12
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, WriteCalculatedECProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleCompany.ecdb", BeFileName(L"SimpleCompany.01.00.ecschema.xml")));
    Utf8String instanceXml = "<Manager xmlns = \"SimpleCompany.01.00\" >"
        "<FirstName>StRiNg - 10002</FirstName>"
        "<LastName>StRiNg - 10002</LastName>"
        "<EmployeeId>10002</EmployeeId>"
        "<LatestEducation>"
        "<Degree>StRiNg - 10007</Degree>"
        "<Year>10010</Year>"
        "</LatestEducation>"
        "<EducationHistory>"
        "<EducationStruct>"
        "<Degree>StRiNg - 10008</Degree>"
        "<GPA>10004.5</GPA>"
        "</EducationStruct>"
        "<EducationStruct>"
        "<Degree>StRiNg - 10009</Degree>"
        "<Year>10011</Year>"
        "<GPA>10005</GPA>"
        "</EducationStruct>"
        "<EducationStruct>"
        "<Year>10012</Year>"
        "<GPA>10005.5</GPA>"
        "</EducationStruct>"
        "</EducationHistory>"
        "<FullName>StRiNg - 10002 StRiNg - 10002</FullName>"
        "</Manager>";
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schemaReadContext, *schema, &schema);
    IECInstancePtr instance;
    InstanceReadStatus status = IECInstance::ReadFromXmlString(instance, instanceXml.c_str(), *instanceContext);
    ASSERT_TRUE(InstanceReadStatus::Success == status);

    Savepoint s(m_ecdb, "Populate");
    ECClassCR ecClass = instance->GetClass();
    ECInstanceInserter inserter(m_ecdb, ecClass, nullptr);
    ECInstanceKey id;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(id, *(instance.get())));
    instance->SetInstanceId("");
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(id, *(instance.get())));
    instance->SetInstanceId("");
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(id, *(instance.get())));
    s.Commit();
    m_ecdb.SaveChanges();

    SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    ECSqlStatement ecStatement;
    ECSqlStatus prepareStatus = ecStatement.Prepare(m_ecdb, ecSql.GetUtf8CP());
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    bvector<IECInstancePtr> instances;
    ECInstanceECSqlSelectAdapter adapter(ecStatement);
    while (BE_SQLITE_ROW == ecStatement.Step())
        {
        IECInstancePtr newInstance = adapter.GetInstance();
        ASSERT_TRUE(ECDbTestUtility::CompareECInstances(*newInstance, *instance));
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                     06/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PointsMappedToSharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("pointsmappedtosharedcolumns.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Prop1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1'>"
        "        <ECCustomAttributes>"
        "           <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "           </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Prop2' typeName='double' />"
        "        <ECProperty propertyName='Center' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub1(Prop1,Center) VALUES(1.1,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(1, DPoint3d::From(1.0, 2.0, 3.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECClassId sub1ClassId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("Sub1")->GetId();
    Utf8String expectedNativeSql;
    expectedNativeSql.Sprintf("INSERT INTO [ts_Base] ([Id],[Prop1],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,1.1,%s);INSERT INTO [ts_Sub1] ([BaseId],[js2],[js3],[js4],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,:_ecdb_ecsqlparam_ix1_col1,:_ecdb_ecsqlparam_ix1_col2,:_ecdb_ecsqlparam_ix1_col3,%s)", sub1ClassId.ToString().c_str(), sub1ClassId.ToString().c_str());
    ASSERT_STREQ(expectedNativeSql.c_str(), stmt.GetNativeSql());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Center.X, Center.Y, Center.Z FROM ts.Sub1 LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1.0, stmt.GetValueDouble(0));
    ASSERT_EQ(2.0, stmt.GetValueDouble(1));
    ASSERT_EQ(3.0, stmt.GetValueDouble(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Sub1 WHERE Center.X > 0 AND Center.Y > 0 AND Center.Z > 0"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindZeroBlob)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("bindzeroblob.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Prop1" typeName="Binary" />
                                <ECProperty propertyName="Prop2" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Prop3" typeName="String" />
                                <ECArrayProperty propertyName="Prop4" typeName="Binary" />
                                <ECStructArrayProperty propertyName="Prop5" typeName="ecdbmap:DbIndex" />
                                <ECProperty propertyName="Prop1_Overflow" typeName="Binary" />
                                <ECProperty propertyName="Prop2_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element(Prop1,Prop2,Prop3,Prop4,Prop5,Prop1_Overflow,Prop2_Overflow) VALUES(?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, 10)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(2, 10)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(3, 10)) << stmt.GetECSql() << "BindZeroBlob against string prop is ok as blob and strings are compatible in SQLite";
    ASSERT_EQ(ECSqlStatus::Error, stmt.BindZeroBlob(4, 10)) << stmt.GetECSql() << "BindZeroBlob against prim array is never allowed";
    ASSERT_EQ(ECSqlStatus::Error, stmt.BindZeroBlob(5, 10)) << stmt.GetECSql() << "BindZeroBlob against struct array is never allowed";
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(6, 10)) << stmt.GetECSql() << "BindZeroBlob against property mapped to overflow col is never allowed";
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(7, 10)) << stmt.GetECSql() << "BindZeroBlob against property mapped to overflow col is never allowed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BlobIO)
    {
    auto assertBlobIO = [] (ECDbCR ecdb, Utf8CP className, Utf8CP accessString)
        {
        ECClassCP ecClass = ecdb.Schemas().GetClass("ECSqlTest", className);
        ASSERT_TRUE(ecClass != nullptr) << className;

        std::vector<Utf8String> expectedBlobChunks {"Hello",", ", "world"};
        const int expectedBlobSize = 13;

        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO %s(%s) VALUES(?)", ecClass->GetECSqlName().c_str(), accessString);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, expectedBlobSize)) << stmt.GetECSql();
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        BlobIO io;
        ASSERT_EQ(SUCCESS, ecdb.OpenBlobIO(io, *ecClass, accessString, key.GetInstanceId(), true));
        ASSERT_TRUE(io.IsValid());
        ASSERT_EQ(expectedBlobSize, io.GetNumBytes());

        int offset = 0;
        for (Utf8StringCR blobChunk : expectedBlobChunks)
            {
            ASSERT_EQ(BE_SQLITE_OK, io.Write(blobChunk.c_str(), (int) blobChunk.size(), offset));
            offset += (int) blobChunk.size();
            }
        //add trailing \0
        Utf8Char nullChar('\0');
        ASSERT_EQ(BE_SQLITE_OK, io.Write(&nullChar, 1, offset));
        ASSERT_EQ(BE_SQLITE_OK, io.Close());
        ASSERT_FALSE(io.IsValid());

        //validate result
        ecsql.Sprintf("SELECT %s FROM %s WHERE ECInstanceId=%s", accessString, 
                      ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        int actualBlobSize = -1;
        ASSERT_STREQ("Hello, world", (Utf8CP) stmt.GetValueBlob(0, &actualBlobSize)) << stmt.GetECSql();
        ASSERT_EQ(expectedBlobSize, actualBlobSize) << stmt.GetECSql();

        //validate result via blobio
        ASSERT_EQ(SUCCESS, ecdb.OpenBlobIO(io, *ecClass, accessString, key.GetInstanceId(), false));
        ASSERT_TRUE(io.IsValid());
        ASSERT_EQ(expectedBlobSize, io.GetNumBytes());
        Utf8String actualBlobBuffer;
        actualBlobBuffer.reserve((size_t) expectedBlobSize);
        ASSERT_EQ(BE_SQLITE_OK, io.Read(const_cast<Utf8P> (actualBlobBuffer.data()), expectedBlobSize, 0));
        ASSERT_STREQ("Hello, world", actualBlobBuffer.c_str()) << "BlobIO::Read";
        };


    ASSERT_EQ(SUCCESS, SetupECDb("blobio.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    assertBlobIO(m_ecdb, "P", "Bi");
    assertBlobIO(m_ecdb, "PSA", "PStructProp.bi");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BlobIOForInvalidProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("blobioinvalidcases.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));

    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        const bool expectedToSucceed = prop->GetIsPrimitive() && (prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_Binary || prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_IGeometry);
        if (!expectedToSucceed)
            {
            BlobIO io;
            ASSERT_EQ(ERROR, m_ecdb.OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Not a binary/geometry property";
            ASSERT_FALSE(io.IsValid());
            }
        }
    }

    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        BlobIO io;
        ASSERT_EQ(ERROR, m_ecdb.OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Cannot use BlobIO on ECStructs";
        ASSERT_FALSE(io.IsValid());
        }
    }

    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECDbMap", "ClassMap");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        BlobIO io;
        ASSERT_EQ(ERROR, m_ecdb.OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Cannot use BlobIO on custom attribute classes";
        ASSERT_FALSE(io.IsValid());
        }
    }

    m_ecdb.CloseDb();

    {
    ASSERT_EQ(SUCCESS, SetupECDb("blobioinvalidcases2.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Prop1" typeName="Binary" />
                                <ECProperty propertyName="Prop2" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Prop1_Overflow" typeName="Binary" />
                                <ECProperty propertyName="Prop2_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml")));

    ECInstanceKey key;
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO TestSchema.Element(Prop1,Prop2,Prop1_Overflow,Prop2_Overflow) VALUES(zeroblob(10),?,?,?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, 10)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(2, 10)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(3, 10)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        }

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "Element");
    ASSERT_TRUE(testClass != nullptr);

        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop1", key.GetInstanceId(), true)) << "Binary property not mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop2", key.GetInstanceId(), true)) << "IGeometry property not mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop1_Overflow", key.GetInstanceId(), true)) << "Binary property mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop2_Overflow", key.GetInstanceId(), true)) << "IGeometry property mapped to overflow table";
        }
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ECSqlOnAbstractClassWithOrderbyClause)
    {
    auto assertECSql = [] (Utf8CP ecsql, ECDbCR ecdb, ECSqlStatus sqlStatus = ECSqlStatus::Success, DbResult dbResult = DbResult::BE_SQLITE_DONE)
        {
        ECSqlStatement statement;
        ASSERT_EQ(sqlStatus, statement.Prepare(ecdb, ecsql));
        ASSERT_EQ(dbResult, statement.Step());
        };

    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlonabstractclasswithorderbyclause.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                       "<ECSchema schemaName='TestSchema' alias='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                                                       "        <ECProperty propertyName='P0' typeName='string' />"
                                                                       "        <ECProperty propertyName='P1' typeName='int' />"
                                                                       "    </ECEntityClass>"
                                                                       "    <ECEntityClass typeName='GeometricElement' >"
                                                                       "         <BaseClass>Element</BaseClass>"
                                                                       "    </ECEntityClass>"
                                                                       "</ECSchema>")));

    assertECSql("INSERT INTO TestSchema.GeometricElement(P0, P1) VALUES ('World ', 2)", m_ecdb);
    assertECSql("INSERT INTO TestSchema.GeometricElement(P0, P1) VALUES ('Hello ', 1)", m_ecdb);
    assertECSql("INSERT INTO TestSchema.GeometricElement(P0, P1) VALUES ('!!!', 3)", m_ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT P0 FROM TestSchema.Element ORDER BY P1"));
    int rowCount = 0;
    Utf8CP expectedValues = "Hello World !!!";
    Utf8String actualValues;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        actualValues.append(statement.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedValues, actualValues.c_str()) << statement.GetECSql();
    ASSERT_EQ(3, rowCount);
    statement.Finalize();
    }



//---------------------------------------------------------------------------------
// @bsimethod                             Krischan.Eberle                      02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, OrderByAgainstMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("orderbyonmixin.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECProperty propertyName="BaseProp1" typeName="string" />
                <ECProperty propertyName="BaseProp2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="IOriginSource" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                  </IsMixin>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Origin" typeName="Point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="Geometric2dElement">
                 <BaseClass>Base</BaseClass>
                 <BaseClass>IOriginSource</BaseClass>
                <ECProperty propertyName="TwoDType" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Geometric3dElement">
                 <BaseClass>Base</BaseClass>
                 <BaseClass>IOriginSource</BaseClass>
                <ECProperty propertyName="ThreeDType" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml")));

    {
    int id = 1;
    ECSqlStatement statement1;
    ASSERT_EQ(ECSqlStatus::Success, statement1.Prepare(m_ecdb, "INSERT INTO ts.Geometric2dElement(TwoDType, Code, Origin, BaseProp2) VALUES('Line', ?, ?, ?)"));

    ECSqlStatement statement2;
    ASSERT_EQ(ECSqlStatus::Success, statement2.Prepare(m_ecdb, "INSERT INTO ts.Geometric3dElement(ThreeDType, Code, Origin, BaseProp2) VALUES('Volume', ?, ?, ?)"));

    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(1, 10));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindPoint3d(2, DPoint3d::From(10.0,10.0,10.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement1.Step());
    statement1.Reset();
    statement1.ClearBindings();

    id++;
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(1, 5));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindPoint3d(2, DPoint3d::From(5.0, 5.0, 5.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement2.Step());
    statement2.Reset();
    statement2.ClearBindings();

    id++;
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindPoint3d(2, DPoint3d::From(2.0, 2.0, 2.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement1.Step());
    statement1.Reset();
    statement1.ClearBindings();

    id++;
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindPoint3d(2, DPoint3d::From(1.0, 1.0, 1.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement2.Step());
    statement2.Reset();
    statement2.ClearBindings();
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Origin.X, Origin.Y, Origin.Z, Code FROM ts.IOriginSource ORDER BY Code"));
    std::vector<int> expectedCodes {1, 2, 5, 10};
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        int ix = rowCount;
        rowCount++;
        const int actualCode = statement.GetValueInt(3);
        ASSERT_EQ(expectedCodes[(size_t) ix], actualCode);
        ASSERT_EQ(actualCode, statement.GetValueInt(0));
        ASSERT_EQ(actualCode, statement.GetValueInt(1));
        ASSERT_EQ(actualCode, statement.GetValueInt(2));
        }

    ASSERT_EQ((int) expectedCodes.size(), rowCount);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, UpdateAndDeleteAgainstMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updatedeletemixin.ecdb",
                           SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                          <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                          <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                                          <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                              <ECEntityClass typeName="Model">
                                                 <ECCustomAttributes>
                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                          <MapStrategy>TablePerHierarchy</MapStrategy>
                                                   </ClassMap>
                                                   <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                                                 </ECCustomAttributes>
                                                  <ECProperty propertyName="Name" typeName="string" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="IIsGeometricModel" modifier="Abstract">
                                                 <ECCustomAttributes>
                                                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                                                        <AppliesToEntityClass>Model</AppliesToEntityClass>
                                                    </IsMixin>"
                                                 </ECCustomAttributes>
                                                  <ECProperty propertyName="Is2d" typeName="boolean"/>
                                                  <ECProperty propertyName="SupportedGeometryType" typeName="string" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Model2d">
                                                    <BaseClass>Model</BaseClass>
                                                    <BaseClass>IIsGeometricModel</BaseClass>
                                                  <ECProperty propertyName="Origin2d" typeName="Point2d" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Model3d">
                                                    <BaseClass>Model</BaseClass>
                                                    <BaseClass>IIsGeometricModel</BaseClass>
                                                  <ECProperty propertyName="Origin3d" typeName="Point3d" />
                                              </ECEntityClass>
                                          </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    m_ecdb.SaveChanges();

    std::vector<ECInstanceKey> model2dKeys, model3dKeys, element2dKeys, element3dKeys;
    //INSERT
    {
    ECInstanceKey key;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model2d(Name,Is2d,SupportedGeometryType,Origin2d) VALUES(?,true,'Line',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Main2d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(2, DPoint2d::From(1.0, 1.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model2dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Detail2d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(2, DPoint2d::From(1.5, 1.5)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model2dKeys.push_back(key);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model3d(Name,Is2d,SupportedGeometryType,Origin3d) VALUES(?,false,'Solid',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Main3d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(2.0, 2.0, 2.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model3dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Detail3d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(2.5, 2.5, 2.5)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model3dKeys.push_back(key);
    }
    m_ecdb.SaveChanges();
    {
    //Select models via mixin
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Is2d, SupportedGeometryType FROM ts.IIsGeometricModel"));
    int rowCount2d = 0;
    int rowCount3d = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId actualClassId = stmt.GetValueId<ECClassId>(1);
        if (actualClassId == model2dKeys[0].GetClassId())
            {
            rowCount2d++;
            ASSERT_TRUE(stmt.GetValueBoolean(2)) << stmt.GetECSql();
            ASSERT_STREQ("Line", stmt.GetValueText(3)) << stmt.GetECSql();
            }
        else if (actualClassId == model3dKeys[0].GetClassId())
            {
            rowCount3d++;
            ASSERT_FALSE(stmt.GetValueBoolean(2)) << stmt.GetECSql();
            ASSERT_STREQ("Solid", stmt.GetValueText(3)) << stmt.GetECSql();
            }
        }

    ASSERT_EQ(2, rowCount2d) << stmt.GetECSql();
    ASSERT_EQ(2, rowCount3d) << stmt.GetECSql();
    stmt.Finalize();

    //Select concrete models
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin2d FROM ts.Model2d"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("Line", stmt.GetValueText(2));

        const ECInstanceId actualModelId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualModelName = stmt.GetValueText(1);
        if (actualModelId == model2dKeys[0].GetInstanceId())
            ASSERT_STREQ("Main2d", actualModelName) << stmt.GetECSql();
        else if (actualModelId == model2dKeys[1].GetInstanceId())
            ASSERT_STREQ("Detail2d", actualModelName) << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin3d FROM ts.Model3d"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("Solid", stmt.GetValueText(2));

        const ECInstanceId actualModelId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualModelName = stmt.GetValueText(1);
        if (actualModelId == model3dKeys[0].GetInstanceId())
            ASSERT_STREQ("Main3d", actualModelName) << stmt.GetECSql();
        else if (actualModelId == model3dKeys[1].GetInstanceId())
            ASSERT_STREQ("Detail3d", actualModelName) << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();
    }

    //UPDATE Mixin non-polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.IIsGeometricModel SET SupportedGeometryType='Surface' WHERE Is2d"));
    }

    //UPDATE Mixin polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.IIsGeometricModel SET SupportedGeometryType='Surface' WHERE Is2d"));
    }

    //DELETE Mixin non-polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.IIsGeometricModel WHERE Is2d=False"));
    }

    //DELETE Mixin polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ts.IIsGeometricModel WHERE Is2d=False"));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ECInstanceIdConversion)
    {
    //ToString
    ECInstanceId ecInstanceId(UINT64_C(123456789));
    Utf8CP expectedInstanceId = "123456789";
    Utf8Char actualInstanceId[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    ecInstanceId.ToString(actualInstanceId);
    EXPECT_STREQ(expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValue();

    ecInstanceId = ECInstanceId(UINT64_C(0));
    expectedInstanceId = "0";
    actualInstanceId[0] = '\0';
    ecInstanceId.ToString(actualInstanceId);
    EXPECT_STREQ(expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValueUnchecked();

    //FromString
    Utf8CP instanceId = "123456789";
    ECInstanceId expectedECInstanceId(UINT64_C(123456789));
    ECInstanceId actualECInstanceId;
    EXPECT_EQ(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));
    EXPECT_EQ(expectedECInstanceId.GetValue(), actualECInstanceId.GetValue()) << "Unexpected ECInstanceId parsed from InstanceId " << instanceId;

    instanceId = "0";
    expectedECInstanceId = ECInstanceId(UINT64_C(0));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));

    instanceId = "0000";
    expectedECInstanceId = ECInstanceId(UINT64_C(0));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));

    instanceId = "-123456";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with negative number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "-12345678901234";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with negative number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    //now test with invalid instance ids
    BeTest::SetFailOnAssert(false);

    instanceId = "0x75BCD15";
    expectedECInstanceId = ECInstanceId(UINT64_C(123456789));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with hex formatted number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "i-12345";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId starting with i- '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "1234a123";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "Non-numeric InstanceId '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "blabla";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "Non-numeric InstanceId '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    BeTest::SetFailOnAssert(true);
    }
//---------------------------------------------------------------------------------
// Verifies correct ECSQL parsing on Android
// @bsimethod                              Affan.Khan                       10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ECSqlParseTreeFormatter_ParseAndFormatECSqlParseNodeTree)
    {
    auto AssertParseECSql = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        Utf8String parseTree;
        ASSERT_EQ(SUCCESS, ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, ecdb, ecsql)) << "Failed to parse ECSQL";
        };

    ECDb ecdb; // only needed for issue listener, doesn't need to represent a file on disk
    AssertParseECSql(ecdb, "SELECT '''' FROM stco.Hardware");
    AssertParseECSql(ecdb, "SELECT 'aa', '''', b FROM stco.Hardware WHERE Name = 'a''b'");
    AssertParseECSql(ecdb, "SELECT _Aa, _bC, _123, Abc, a123, a_123, a_b, _a_b_c FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql(ecdb, "SELECT * FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql(ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo]");
    AssertParseECSql(ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo] WHERE [Name] = 'HelloWorld'");
    AssertParseECSql(ecdb, "Select EQUIP_NO From only appdw.Equipment where EQUIP_NO = '50E-101A' ");
    AssertParseECSql(ecdb, "INSERT INTO [V8TagsetDefinitions].[STRUCTURE_IL1] ([VarFixedStartZ], [DeviceID1], [ObjectType], [PlaceMethod], [CopyConstrDrwToProj]) VALUES ('?', '-E1-1', 'SGL', '1', 'Y')");
    AssertParseECSql(ecdb, "INSERT INTO [V8TagsetDefinitions].[grid__x0024__0__x0024__CB_1] ([CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457],[CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454]) VALUES ('', '1.1', '', '', '', '2.2', '', '', '', '2.5', '', '', '', '2.5', '', '', '', '2.1', '', '', '', 'E.3', '', '', '', 'B.4', '', '', '', 'D.4', '', '')");
    }


END_ECDBUNITTESTS_NAMESPACE
