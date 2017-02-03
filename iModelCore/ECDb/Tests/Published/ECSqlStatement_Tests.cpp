/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatement_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"
#include "NestedStructArrayTestSchemaHelper.h"
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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, UnionTests)
    {
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    int rowCount;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM (SELECT CompanyName FROM ECST.Supplier UNION ALL SELECT CompanyName FROM ECST.Shipper)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    int count = stmt.GetValueInt(0);
    EXPECT_EQ(6, count);
    stmt.Finalize();

    //Select Statement containing Union All Clause and also Order By clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CompanyName, Phone FROM ECST.Supplier UNION ALL SELECT CompanyName, Phone FROM ECST.Shipper ORDER BY Phone"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT City FROM ECST.Supplier UNION SELECT City FROM ECST.Customer ORDER BY City"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT City FROM ECST.Supplier UNION ALL SELECT City FROM ECST.Customer ORDER BY City"));
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
    ASSERT_EQ(0, ecdb.AddFunction(func));
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "Select POW(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Count(*), AVG(Phone) FROM (SELECT Phone FROM ECST.Supplier UNION ALL SELECT Phone FROM ECST.Customer)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ(7, stmt.GetValueInt(0));
    ASSERT_EQ(1400, stmt.GetValueInt(1));
    stmt.Finalize();

    //Use GROUP BY clause in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*), Phone FROM (SELECT ECClassId, Phone FROM ECST.Supplier UNION ALL SELECT ECClassId, Phone FROM ECST.Customer) GROUP BY ECClassId ORDER BY Phone"));

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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CompanyName FROM ECST.Supplier EXCEPT SELECT CompanyName FROM ECST.Shipper"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ContactTitle FROM ECST.Customer EXCEPT SELECT ContactTitle FROM ECST.Supplier"));
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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CompanyName FROM ECST.Supplier INTERSECT SELECT CompanyName FROM ECST.Shipper ORDER BY CompanyName"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ContactTitle FROM ECST.Supplier INTERSECT SELECT ContactTitle FROM ECST.Customer ORDER BY ContactTitle"));
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

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                 01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IsNull)
    {
    ECDbCR ecdb = SetupECDb("ecsqlisnull.ecdb", SchemaItem(
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
                              <SharedColumnCount>100</SharedColumnCount>
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
        </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    std::vector<Utf8CP> testClassNames {"Class_NoSharedCols", "Class_SharedCols_NoOverflow", "Class_SharedCols_Overflow"};


    //*** all values null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(ECInstanceId) VALUES(NULL)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct, Struct.struct_array, Struct_Array, "
                      "I, Geom, P2D, P2D.X, P2D.Y, P3D, P3D.X, P3D.Y, P3D.Z, "
                      "Struct.pstruct, Struct.pstruct.i, Struct.pstruct.geom, Struct.pstruct.p2d, Struct.pstruct.p2d.X, Struct.pstruct.p2d.Y, "
                      "Struct.pstruct.p3d, Struct.pstruct.p3d.X, Struct.pstruct.p3d.Y, Struct.pstruct.p3d.Z, "
                      "Struct.l_array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetECInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            ASSERT_TRUE(stmt.IsValueNull(i)) << "no values bound to " << stmt.GetECSql();
            }

        IECSqlStructValue const& structVal = stmt.GetValueStruct(0);
        ASSERT_EQ((int) ecdb.Schemas().GetECClass("TestSchema", "MyStruct")->GetPropertyCount(), structVal.GetMemberCount());
        for (int i = 0; i < structVal.GetMemberCount(); i++)
            {
            ASSERT_TRUE(structVal.GetValue(i).IsNull());
            }

        IECSqlArrayValue const& structMemberStructArrayVal = stmt.GetValueArray(1);
        ASSERT_EQ(0, structMemberStructArrayVal.GetArrayLength());

        IECSqlArrayValue const& structArrayVal = stmt.GetValueArray(2);
        ASSERT_EQ(0, structArrayVal.GetArrayLength());
        }

    //*** array with two null elements
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(L_Array, Struct.l_array, Struct.struct_array, Struct_Array) "
                      "VALUES(?,?,?,?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

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
                      testClassName, key.GetECInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            IECSqlValue const& val = stmt.GetValue(i);
            ASSERT_FALSE(val.IsNull()) << i << " " << stmt.GetECSql();
            IECSqlArrayValue const& arrayVal = val.GetArray();
            ASSERT_EQ(2, arrayVal.GetArrayLength());
            for (IECSqlValue const* elementVal : arrayVal)
                {
                ASSERT_TRUE(elementVal != nullptr) << i << " " << stmt.GetECSql();
                ASSERT_TRUE(elementVal->IsNull()) << i << " " << stmt.GetECSql();

                if (val.GetColumnInfo().GetDataType().IsStructArray())
                    {
                    IECSqlStructValue const& structVal = elementVal->GetStruct();
                    ASSERT_EQ(val.GetColumnInfo().GetProperty()->GetAsStructArrayProperty()->GetStructElementType().GetPropertyCount(), structVal.GetMemberCount());
                    for (int j = 0; j < structVal.GetMemberCount(); j++)
                        {
                        ASSERT_TRUE(structVal.GetValue(j).IsNull());
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
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

        IECSqlBinder& elementBinder = stmt.GetBinder(1).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["struct_array"].BindNull());
        
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetECInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

        ASSERT_FALSE(stmt.IsValueNull(0));
        IECSqlArrayValue const& arrayVal = stmt.GetValueArray(0);
        ASSERT_EQ(1, arrayVal.GetArrayLength());
        IECSqlValue const* elementVal = *arrayVal.begin();
        ASSERT_TRUE(elementVal != nullptr) << stmt.GetECSql();
        ASSERT_TRUE(elementVal->IsNull()) << stmt.GetECSql();

        IECSqlStructValue const& structVal = elementVal->GetStruct();
        ASSERT_EQ(stmt.GetColumnInfo(0).GetProperty()->GetAsStructArrayProperty()->GetStructElementType().GetPropertyCount(), structVal.GetMemberCount());
        for (int i = 0; i < structVal.GetMemberCount(); i++)
            {
            ASSERT_TRUE(structVal.GetValue(i).IsNull());
            }
        }

    //*** nested struct being partially set
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(Struct_Array) VALUES(?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

        IECSqlBinder& arrayElementBinder = stmt.GetBinder(1).AddArrayElement();
        //Set StructArray[0].struct_array[0].i=3
        IECSqlBinder& nestedArrayElementBinder = arrayElementBinder["struct_array"].AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, nestedArrayElementBinder["i"].BindInt(3));

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetECInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

        ASSERT_FALSE(stmt.IsValueNull(0));
        IECSqlArrayValue const& arrayVal = stmt.GetValueArray(0);
        ASSERT_EQ(1, arrayVal.GetArrayLength());
        IECSqlValue const* elementVal = *arrayVal.begin();
        ASSERT_TRUE(elementVal != nullptr) << stmt.GetECSql();
        ASSERT_FALSE(elementVal->IsNull()) << stmt.GetECSql();

        IECSqlStructValue const& structArrayElementVal = elementVal->GetStruct();
        ASSERT_EQ(stmt.GetColumnInfo(0).GetProperty()->GetAsStructArrayProperty()->GetStructElementType().GetPropertyCount(), structArrayElementVal.GetMemberCount());
        for (int i = 0; i < structArrayElementVal.GetMemberCount(); i++)
            {
            IECSqlValue const& memberVal = structArrayElementVal.GetValue(i);
            if (memberVal.GetColumnInfo().GetProperty()->GetName().Equals("struct_array"))
                {
                for (int j = 0; j < memberVal.GetStruct().GetMemberCount(); j++)
                    {
                    IECSqlValue const& nestedMemberVal = memberVal.GetStruct().GetValue(j);
                    if (nestedMemberVal.GetColumnInfo().GetProperty()->GetName().Equals("i"))
                        ASSERT_EQ(3, nestedMemberVal.GetInt());
                    else
                        ASSERT_TRUE(nestedMemberVal.IsNull());
                    }
                }
            else
                ASSERT_TRUE(memberVal.IsNull()) << i;
            }
        }
    
    // points partially null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(P2D.X, P3D.Y, Struct.pstruct.p2d.X, Struct.pstruct.p3d.Y) VALUES(?,?,?,?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, 3.14)) << ecsql.c_str();

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT P2D, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, Struct.pstruct.p2d.X, Struct.pstruct.p2d.Y, Struct.pstruct.p3d.X, Struct.pstruct.p3d.Y, Struct.pstruct.p3d.Z FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetECInstanceId().ToString().c_str());

        std::set<Utf8String> notNullItems {"P2D", "P2D.X", "P3D.Y", "Struct.pstruct.p2d.X", "Struct.pstruct.p3d.Y"};
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
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
// @bsimethod                                      Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForPoints)
    {
    const int rowCountPerClass = 3;
    ECDbCR ecdb = SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), rowCountPerClass);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial LIMIT 1"));
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
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << ecsql;

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
    ECDbCR ecdb = SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), rowCountPerClass);

    ECClassCP structType = ecdb.Schemas().GetECClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(structType != nullptr);

    auto assertColumn = [] (ECClassCR expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsStruct() && &colVal.GetColumnInfo().GetProperty()->GetAsStructProperty()->GetType() == &expected);
        
        if (checkIsNull)
            ASSERT_TRUE(colVal.IsNull());

        IECSqlStructValue const& structColVal = colVal.GetStruct();
        const int actualStructMemberCount = structColVal.GetMemberCount();
        ASSERT_EQ((int) expected.GetPropertyCount(), actualStructMemberCount);

        for (int i = 0; i < actualStructMemberCount; i++)
            {
            IECSqlValue const& memberVal = structColVal.GetValue(i);
            if (checkIsNull)
                ASSERT_TRUE(memberVal.IsNull());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.PSA LIMIT 1"));
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
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << ecsql;

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
    ECDbCR ecdb = SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), rowCountPerClass);

    auto assertColumn = [] (PrimitiveType expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsPrimitiveArray());
        ASSERT_EQ(expected, colVal.GetColumnInfo().GetDataType().GetPrimitiveType());
        if (checkIsNull)
            {
            ASSERT_TRUE(colVal.IsNull());
            ASSERT_EQ(0, colVal.GetArray().GetArrayLength());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CAST(NULL AS TIMESTAMP[]) FROM ecsql.PSA LIMIT 1"));
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
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << ecsql;

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
    ECDbCR ecdb = SetupECDb("nullliterals.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), rowCountPerClass);

    ECClassCP structType = ecdb.Schemas().GetECClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(structType != nullptr);

    auto assertColumn = [] (ECClassCR expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsStructArray());
        ASSERT_TRUE(&colVal.GetColumnInfo().GetProperty()->GetAsStructArrayProperty()->GetStructElementType() == &expected);
        if (checkIsNull)
            {
            ASSERT_TRUE(colVal.IsNull());
            ASSERT_EQ(0, colVal.GetArray().GetArrayLength());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CAST(NULL AS ecsql.PStruct[]) FROM ecsql.PSA LIMIT 1"));
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
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << ecsql;

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
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, NestedSelectStatementsTests)
    {
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ProductName, UnitPrice FROM ECST.Product WHERE UnitPrice > (SELECT AVG(UnitPrice) From ECST.Product) AND UnitPrice < 500"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT ProductName From ECST.Product WHERE UnitPrice = ?"));
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.BindDouble(1, stmt.GetValueDouble(1))) << "Binding Double value failed";
    ASSERT_TRUE(selectStmt.Step() == BE_SQLITE_ROW);
    ASSERT_STREQ(stmt.GetValueText(0), selectStmt.GetValueText(0));
    stmt.Finalize();

    //Using GetECClassId in Nested Select statement
    ECClassId supplierClassId = ecdb.Schemas().GetECClassId("ECST", "Supplier", ResolveSchema::BySchemaAlias);
    ECClassId customerClassId = ecdb.Schemas().GetECClassId("ECST", "Customer", ResolveSchema::BySchemaAlias);
    ECClassId firstClassId = std::min<ECClassId>(supplierClassId, customerClassId);
    ECClassId secondClassId = std::max<ECClassId>(supplierClassId, customerClassId);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, COUNT(*) FROM (SELECT ECClassId FROM ECST.Supplier UNION ALL SELECT ECClassId FROM ECST.Customer) GROUP BY ECClassId ORDER BY ECClassId"));
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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement stmt;
    //Using Predicate function in nexted select statement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice IN (SELECT UnitPrice FROM ECST.Product WHERE UnitPrice < (SELECT AVG(UnitPrice) FROM ECST.Product WHERE ProductAvailable))"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(223, (int) stmt.GetValueDouble(0));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    stmt.Finalize();

    //Using NOT operator with predicate function in Nested Select statement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice IN (SELECT UnitPrice FROM ECST.Product WHERE UnitPrice > (SELECT AVG(UnitPrice) FROM ECST.Product WHERE NOT ProductAvailable))"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(619, (int) stmt.GetValueDouble(0));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                             Krischan.Eberle                        12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ParametersInNestedSelectStatement)
    {
    const int rowCountPerClass = 3;
    ECDbR ecdb = SetupECDb("parametersinnestedselect.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), rowCountPerClass);


    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ecsql.PSA WHERE L=123456789 AND I IN (SELECT I FROM ecsql.P WHERE I=123)"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(rowCountPerClass, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ecsql.PSA WHERE L=? AND I IN (SELECT I FROM ecsql.P WHERE I=?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(L,S) VALUES(314,'Test PSA 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey1)) << stmt.GetECSql();
    stmt.Finalize();

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO ecsql.P(MyPSA.Id,S) VALUES(%s,'Test P')", psaKey1.GetECInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(pKey)) << stmt.GetECSql();
    stmt.Finalize();

    ecsql.Sprintf("INSERT INTO ecsql.PSA(L,I,S) VALUES(314,%s,'Test PSA 2')", pKey.GetECInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey2)) << stmt.GetECSql();
    stmt.Finalize();

    ecdb.SaveChanges();
    }

    {
    Utf8String ecsqlWithoutParams;
    ecsqlWithoutParams.Sprintf("SELECT ECInstanceId FROM ecsql.PSA WHERE L=314 AND I IN (SELECT ECInstanceId FROM ecsql.P WHERE MyPSA.Id=%s)", psaKey1.GetECInstanceId().ToString().c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsqlWithoutParams.c_str())) << ecsqlWithoutParams.c_str();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetECInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.PSA WHERE L=? AND I IN (SELECT ECInstanceId FROM ecsql.P WHERE MyPSA.Id=?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 314)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, psaKey1.GetECInstanceId())) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetECInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT PSA.ECInstanceId FROM ecsql.PSA, ecsql.P WHERE PSA.L=? AND PSA.I=P.ECInstanceId AND P.MyPSA.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 314)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, psaKey1.GetECInstanceId())) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetECInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, GroupByClauseTests)
    {
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    Utf8CP expectedProductsNames;
    Utf8String actualProductsNames;
    double expectedSumOfAvgPrices;
    double actualSumOfAvgPrices;
    ECSqlStatement stmt;
    //use of simple GROUP BY clause to find AVG(Price) from the Product table
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product GROUP BY ProductName ORDER BY ProductName"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product GROUP BY ProductName Having AVG(UnitPrice)>300.00 ORDER BY ProductName"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice<500 GROUP BY ProductName Having AVG(UnitPrice)>200.00 ORDER BY ProductName"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ProductName, AVG(UnitPrice), COUNT(ProductName) FROM ECST.Product GROUP BY ProductName, UnitPrice HAVING COUNT(ProductName)>1"));
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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT AVG(Phone) FROM ECST.Customer GROUP BY PersonName"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(1650, statement.GetValueInt(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT Country FROM ECST.Customer GROUP BY PersonName"));
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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100*5, true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+1+ECClassId, true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000/5, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+2+ECClassId, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000+100*5, true, 'LCD')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductName FROM ECST.Product WHERE UnitPrice=100+2+ECClassId"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Table", statement.GetValueText(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice>ECClassId AND ProductName='Chair'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+ECClassId AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+ECClassId AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice=100+3+ECClassId OR ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, WrapWhereClauseInParams)
    {
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' OR Company='ABC'"));
    Utf8String nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE ([Customer].[Country] = 'USA' OR [Customer].[Company] = 'ABC')") != nativeSql.npos);
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' AND Company='ABC'"));
    nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE [Customer].[Country] = 'USA' AND [Customer].[Company] = 'ABC'") != nativeSql.npos);
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' OR Country='DUBAI' AND ContactTitle='AM'"));
    nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE ([Customer].[Country] = 'USA' OR [Customer].[Country] = 'DUBAI' AND [Customer].[ContactTitle] = 'AM')") != nativeSql.npos);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicDelete_SharedTable)
    {
    ECDbR ecdb = SetupECDb("PolymorphicDeleteSharedTable.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(ecdb, true);

    ASSERT_FALSE(ecdb.TableExists("nsat_DerivedA"));
    ASSERT_FALSE(ecdb.TableExists("nsat_DoubleDerivedA"));
    ASSERT_FALSE(ecdb.TableExists("nsat_DoubleDerivedC"));

    //Delete all Instances of the base class, all the structArrays and relationships should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    bvector<Utf8String> tableNames = {"ClassA", "BaseHasDerivedA", "DerivedBHasChildren"};

    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, selectSql.c_str())) << "Prepare failed for " << selectSql.c_str();
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
    SchemaItem testSchema(NestedStructArrayTestSchemaHelper::s_testSchemaXml, true);
    ECDbR ecdb = SetupECDb("PolymorphicDeleteTest.ecdb", testSchema);
    ASSERT_TRUE(ecdb.IsDbOpen());

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(ecdb, false);

    //Delete all Instances of the base class, all the structArrays should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    bvector<Utf8String> tableNames = {"ClassA" , "DerivedA", "DerivedB", "DoubleDerivedA", "DoubleDerivedB", "DoubleDerivedC"};

    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, selectSql.c_str())) << "Prepare failed for " << selectSql.c_str();
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
    SchemaItem testSchema("<?xml version='1.0' encoding='utf-8' ?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECEntityClass typeName='A' >"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "</ECSchema>", false, "");
    ECDbR ecdb = SetupECDb("PolymorphicDeleteTest.ecdb", testSchema);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceId fi1Id;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecdbf.ExternalFileInfo(ECInstanceId, Name, Size, RootFolder, RelativePath) VALUES(2, 'testfile.txt', 123, 1, 'myfolder')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    fi1Id = key.GetECInstanceId();
    }

    BeBriefcaseBasedId fi2Id;
    {
    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(L"StartupCompany.json");
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    fi2Id = embeddedFileTable.Import(&stat, "embed1", testFilePath.GetNameUtf8().c_str(), "JSON");
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(fi2Id.IsValid());
    }
    ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ecdbf.FileInfo WHERE ECInstanceId=?"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi1Id));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT NULL FROM ecdbf.FileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi1Id));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi2Id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi2Id));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT NULL FROM ecdbf.FileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fi1Id));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
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
    SchemaItem testSchema(NestedStructArrayTestSchemaHelper::s_testSchemaXml, true);
    ECDbR ecdb = SetupECDb("PolymorphicUpdateTest.ecdb", testSchema);
    ASSERT_TRUE(ecdb.IsDbOpen());

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(ecdb, false);

    //Updates the instances of ClassA
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ecdb.SaveChanges();

    bvector<Utf8String> tableNames = {"ClassA", "DerivedA", "DerivedB", "DoubleDerivedA", "DoubleDerivedB", "DoubleDerivedC"};

    Utf8CP expectedValue = "UpdatedValue";
    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectECSql = "SELECT I,T FROM nsat_";
        selectECSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, selectECSql.c_str()));
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
    ECDbR ecdb = SetupECDb("PolymorphicUpdateSharedTable.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(ecdb, true);

    //Updates the instances of ClassA all the Derived Classes Properties values should also be changed. 
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId,ECClassId,I,T FROM nsat.ClassA ORDER BY ECInstanceId"));
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
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM ECST.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(9, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ECST.Product WHERE ProductName IN(SELECT ProductName FROM ECST.Product GROUP BY ProductName HAVING COUNT(ProductName)>2 AND ProductName IN(SELECT ProductName FROM ECST.Product WHERE UnitPrice >500))"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM ECST.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, UpdateWithNestedSelectStatments)
    {
    ECDbR ecdb = SetupECDb("ECSqlStatementTests.ecdb", BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(ecdb);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ECST.Product SET ProductName='Laptop' WHERE ProductName IN(SELECT ProductName FROM ECST.Product GROUP BY ProductName HAVING COUNT(ProductName)>2 AND ProductName IN(SELECT ProductName FROM ECST.Product WHERE UnitPrice >500))"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM ECST.Product WHERE ProductName='Laptop'"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(3, stmt.GetValueInt(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                    02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertStructArray)
    {
    ECDbR ecdb = SetupECDb("PolymorphicUpdateTest.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml"));

    ECInstanceList instanceList = NestedStructArrayTestSchemaHelper::CreateECInstances(ecdb, 1, "ClassP");

    Utf8String inXml, outXml;
    for (IECInstancePtr inst : instanceList)
        {
        ECInstanceInserter inserter(ecdb, inst->GetClass(), nullptr);
        ASSERT_TRUE(inserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*inst, true));
        inst->WriteToXmlString(inXml, true, true);
        inXml += "\r\n";
        }

    bvector<IECInstancePtr> out;
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        out.push_back(inst);
        inst->WriteToXmlString(outXml, true, true);
        outXml += "\r\n";
        }

    ASSERT_EQ(instanceList.size(), out.size());
    ASSERT_TRUE(inXml == outXml);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                    02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, DeleteStructArray)
    {
    ECDbR ecdb = SetupECDb("PolymorphicUpdateTest.ecdb", BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml"));

    auto in = NestedStructArrayTestSchemaHelper::CreateECInstances(ecdb, 1, "ClassP");

    int insertCount = 0;
    for (auto inst : in)
        {
        ECInstanceInserter inserter(ecdb, inst->GetClass(), nullptr);
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*inst));
        insertCount++;
        }

    ECClassCP classP = ecdb.Schemas().GetECClass("NestedStructArrayTest", "ClassP");
    ASSERT_TRUE(classP != nullptr);

    ECInstanceDeleter deleter(ecdb, *classP, nullptr);

    int deleteCount = 0;
    for (auto& inst : in)
        {
        ASSERT_EQ(BE_SQLITE_OK, deleter.Delete(*inst));
        deleteCount++;
        }

    //Verify Inserted Instance have been deleted.
    bvector<IECInstancePtr> out;
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    ASSERT_FALSE(stmt.Step() == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Int64InStructArrays)
    {
    ECDbCR ecdb = SetupECDb("Int64InStructArrays.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECStructClass typeName='MyStruct' >"
        "        <ECProperty propertyName='I' typeName='int' />"
        "        <ECProperty propertyName='I64' typeName='long' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECStructArrayProperty propertyName='StructArrayProp' typeName='MyStruct' />"
        "    </ECEntityClass>"
        "</ECSchema>"));

    ASSERT_TRUE(ecdb.IsDbOpen());

    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Foo(StructArrayProp) VALUES(?)"));
    IECSqlBinder& arrayElementBinder = stmt.GetBinder(1).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["I"].BindInt(123456));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["I64"].BindInt64(id.GetValue()));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    Statement rawStmt;
    ASSERT_EQ(BE_SQLITE_OK, rawStmt.Prepare(ecdb, "SELECT StructArrayProp FROM ts_Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(BE_SQLITE_OK, rawStmt.BindId(1, key.GetECInstanceId()));
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
    ECDbCR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 3, ECDb::OpenParams(Db::OpenMode::Readonly));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, MyPSA.Id, MyPSA.RelECClassId, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z FROM ecsql.P LIMIT 1"));
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.PSAHasPSA_NN LIMIT 1"));
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    auto ecsql = "SELECT c1.ECInstanceId, c2.ECInstanceId, c1.ECClassId, c2.ECClassId FROM ecsql.PSA c1, ecsql.P c2 LIMIT 1";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

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
    ECDbCR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 3, ECDb::OpenParams(Db::OpenMode::Readonly));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT MyPSA, P2D, P3D FROM ecsql.P LIMIT 1"));
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
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayInsert)
    {
    const int perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    Utf8CP ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(?, ?)";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

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
// @bsiclass                                     Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InvalidBindArrayCalls)
    {
    ECDbCR ecdb = SetupECDb("invalidbindarraycalls.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ecsql.PSA(I_Array, PStruct_Array) VALUES(?,?)"));

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
    const int perClassRowCount = 2;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    Utf8CP ecsql = "UPDATE  ONLY ecsql.PSA SET L = ?,  PStruct_Array = ? WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
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
    const auto perClassRowCount = 2;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
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
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    ECInstanceKey pKey;
    ECInstanceKey psaKey;

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES(NULL)");
    ASSERT_EQ(ECSqlStatus::Success, stat);
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(pKey));

    statement.Finalize();
    stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PSA (ECInstanceId) VALUES(NULL)");
    ASSERT_EQ(ECSqlStatus::Success, stat);
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(psaKey));
    ecdb.SaveChanges();
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, psaKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(2, pKey.GetECInstanceId()));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    ASSERT_EQ(pKey.GetECInstanceId().GetValue(), key.GetECInstanceId().GetValue());
    ecdb.AbandonChanges();
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(1, (int) psaKey.GetECInstanceId().GetValue())) << "Ids cannot be cast to int without potentially losing information. So BindInt cannot be used for ids";
    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(2, (int) pKey.GetECInstanceId().GetValue())) << "Ids cannot be cast to int without potentially losing information. So BindInt cannot be used for ids";
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, psaKey.GetECInstanceId().GetValue()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(2, pKey.GetECInstanceId().GetValue()));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    ASSERT_EQ(pKey.GetECInstanceId().GetValue(), key.GetECInstanceId().GetValue());
    ecdb.AbandonChanges();
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    Utf8Char psaIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    psaKey.GetECInstanceId().ToString(psaIdStr);
    Utf8Char pIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    pKey.GetECInstanceId().ToString(pIdStr);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, psaIdStr, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, pIdStr, IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    ASSERT_EQ(pKey.GetECInstanceId().GetValue(), key.GetECInstanceId().GetValue());
    ecdb.AbandonChanges();
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertNullForECInstanceId)
    {
    ECDbR ecdb = SetupECDb("ecinstanceidbindnull.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto assertSequence = [] (ECDbCR ecdb, BeInt64Id expectedSequenceValue)
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Val FROM be_Local WHERE Name='ec_ecinstanceidsequence'");
        ASSERT_TRUE(stmt != nullptr);
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        ASSERT_EQ(expectedSequenceValue.GetValue(), stmt->GetValueUInt64(0));
        };

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(?)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, ecdb.SaveChanges());
    assertSequence(ecdb, key.GetECInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, ecdb.SaveChanges());
    assertSequence(ecdb, key.GetECInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, ecdb.SaveChanges());
    assertSequence(ecdb, key.GetECInstanceId());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindSourceAndTargetECInstanceId)
    {
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(1111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(2222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(11111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(22222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(1111111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(2222222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement statement;

    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    std::vector<int> expectedIntArray = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<Utf8String> expectedStringArray = {"1", "2", "3", "4", "5", "6", "7", "8"};

    ECInstanceKey ecInstanceKey;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ecsql.PA (I_Array,S_Array) VALUES(:ia,:sa)"));

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
    auto stat = statement.Prepare(ecdb, "SELECT I_Array, S_Array FROM ONLY ecsql.PA WHERE ECInstanceId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);
    statement.BindId(1, ecInstanceKey.GetECInstanceId());

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    IECSqlArrayValue const& intArray = statement.GetValueArray(0);
    size_t expectedIndex = 0;
    for (IECSqlValue const* arrayElement : intArray)
        {
        int actualArrayElement = arrayElement->GetInt();
        ASSERT_EQ(expectedIntArray[expectedIndex], actualArrayElement);
        expectedIndex++;
        }

    IECSqlArrayValue const& stringArray = statement.GetValueArray(1);
    expectedIndex = 0;
    for (IECSqlValue const* arrayElement : stringArray)
        {
        auto actualArrayElement = arrayElement->GetText();
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PA (Dt_Array, DtUtc_Array) VALUES(:dt,:dtutc)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

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
        ASSERT_EQ(expectedStat, arrayElementBinder.BindDateTime(testDate));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindPrimArrayWithOutOfBoundsLength)
    {
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.ABounded (Prim_Array_Bounded) VALUES(?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

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
    const int perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, "INSERT INTO ecsql.ABounded (PStruct_Array_Bounded) VALUES(?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

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
    const int perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    auto testFunction = [this, &ecdb] (Utf8CP insertECSql, bool bindExpectedToSucceed, int structParameterIndex, Utf8CP structValueJson, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueJson, expectedStructValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(ecdb, insertECSql);
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
        stat = statement.Prepare(ecdb, verifySelectECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of verification ECSQL '" << verifySelectECSql << "' failed";
        statement.BindId(1, ecInstanceKey.GetECInstanceId());

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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    //insert some test instances
    auto insertFunction = [this, &ecdb] (ECInstanceKey& ecInstanceKey, Utf8CP insertECSql, int structParameterIndex, Utf8CP structValueToBindJson)
        {
        Json::Value structValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueToBindJson, structValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(ecdb, insertECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of ECSQL '" << insertECSql << "' failed";

        IECSqlBinder& structBinder = statement.GetBinder(structParameterIndex);

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson(bindStatus, statement, structValue, structBinder);

        auto stepStat = statement.Step(ecInstanceKey);
        ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStat) << "Execution of ECSQL '" << insertECSql << "' failed";
        };

    auto testFunction = [this, &ecdb] (Utf8CP updateECSql, int structParameterIndex, Utf8CP structValueJson, int ecInstanceIdParameterIndex, ECInstanceKey ecInstanceKey, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueJson, expectedStructValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(ecdb, updateECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of ECSQL '" << updateECSql << "' failed";

        stat = statement.BindId(ecInstanceIdParameterIndex, ecInstanceKey.GetECInstanceId());
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Binding ECInstanceId to ECSQL '" << updateECSql << "' failed";

        auto& binder = statement.GetBinder(structParameterIndex);

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson(bindStatus, statement, expectedStructValue, binder);
        auto stepStat = statement.Step();

        ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStat);

        statement.Finalize();
        stat = statement.Prepare(ecdb, verifySelectECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of verification ECSQL '" << verifySelectECSql << "' failed";
        statement.BindId(1, ecInstanceKey.GetECInstanceId());

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

    // Create and populate a sample project
    SetupECDb("ecsqlstatementtests.ecdb", schema);
    ASSERT_TRUE(GetECDb().IsDbOpen());

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Person (FullName.[First], FullName.[Last]) VALUES (?,?)"));

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.Person WHERE FullName=?"));
    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.Person WHERE FullName<>?"));
    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetNativeSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.Person WHERE FullName IN (?,?,?)"));

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "UPDATE ts.Person SET FullName.[Last]='Meyer' WHERE FullName=?"));

    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.Person WHERE FullName.[Last]=?"));
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ?, S FROM ecsql.PSA LIMIT 1"));

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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT -?, S FROM ecsql.PSA LIMIT 1"));

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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT -? AS MyId, S FROM ecsql.PSA LIMIT 1"));

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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :i AND S = :s AND L = :i * 1000000 + 456789");
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
    auto stat = statement.Prepare(ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l");
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
    auto stat = statement.Prepare(ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = ?");
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
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :value")) << "VALUE is a reserved word in the ECSQL grammar, so cannot be used without escaping, even in parameter names";
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ecsql.PSA (L,S,I) VALUES (?,?,:[value])"));

    int actualParamIndex = statement.GetParameterIndex("value");
    ASSERT_EQ(3, actualParamIndex);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(actualParamIndex, 300471));

    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(newKey));

    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.PSA WHERE ECInstanceId = :[id]"));
    actualParamIndex = statement.GetParameterIndex("id");
    ASSERT_EQ(1, actualParamIndex);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(actualParamIndex, newKey.GetECInstanceId()));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(newKey.GetECInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NoECClassIdFilterOption)
    {
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=True"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=False"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=0"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=1"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ReadonlyPropertiesAreUpdatable)
    {
    ECDbR ecdb = SetupECDb("ReadonlyPropertiesAreUpdatable.ecdb", 
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
                               "</ECSchema>"),0);

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SubElement(ReadonlyProp1,ReadonlyProp2) VALUES(1,2)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "UPDATE ONLY ts.SubElement SET ReadonlyProp1=10, ReadonlyProp2=20"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ONLY ts.SubElement SET ReadonlyProp1=10, ReadonlyProp2=20 ECSQLOPTIONS ReadonlyPropertiesAreUpdatable"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //verify update worked
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ReadonlyProp1, ReadonlyProp2 FROM ts.SubElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetECInstanceId()));
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT c.Dt_Array FROM ecsql.PSA c LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("Dt_Array", false, "Dt_Array", "PSA", "c", topLevelColumnInfo);
    auto const& topLevelArrayValue = stmt.GetValueArray(0);

    //out of bounds test
    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid()) << "ECSqlStatement::GetColumnInfo (-1) is expected to fail";
    ASSERT_FALSE(stmt.GetColumnInfo(2).IsValid()) << "ECSqlStatement::GetColumnInfo is expected to fail with too large index";
    
    //In array level
    int arrayIndex = 0;
    for (IECSqlValue const* arrayElement : topLevelArrayValue)
        {
        auto const& arrayElementColumnInfo = arrayElement->GetColumnInfo();
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1"));

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
    auto const& topLevelStructValue = stmt.GetValueStruct(0);
    auto const& nestedStructPropColumnInfo = topLevelStructValue.GetValue(0).GetColumnInfo(); //0 refers to first member in SAStructProp which is PStructProp
    AssertColumnInfo("PStructProp", false, "SAStructProp.PStructProp", "SA", nullptr, nestedStructPropColumnInfo);
    auto const& nestedStructValue = topLevelStructValue.GetValue(0).GetStruct();

    //SAStructProp.PStructProp.XXX level
    auto const& firstStructMemberColumnInfo = nestedStructValue.GetValue(0).GetColumnInfo();
    AssertColumnInfo("b", false, "SAStructProp.PStructProp.b", "SA", nullptr, firstStructMemberColumnInfo);

    auto const& secondStructMemberColumnInfo = nestedStructValue.GetValue(1).GetColumnInfo();
    AssertColumnInfo("bi", false, "SAStructProp.PStructProp.bi", "SA", nullptr, secondStructMemberColumnInfo);

    auto const& eighthStructMemberColumnInfo = nestedStructValue.GetValue(8).GetColumnInfo();
    AssertColumnInfo("p2d", false, "SAStructProp.PStructProp.p2d", "SA", nullptr, eighthStructMemberColumnInfo);

    //out of bounds test
    ASSERT_FALSE(nestedStructValue.GetValue(-1).GetColumnInfo().IsValid());
    ASSERT_FALSE(nestedStructValue.GetValue(nestedStructValue.GetMemberCount()).GetColumnInfo().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForStructArrays)
    {
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("SAStructProp", false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);
    auto const& topLevelStructValue = stmt.GetValueStruct(0);

    //out of bounds test
    {
    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid());
    ASSERT_FALSE(stmt.GetColumnInfo(2).IsValid());
    }

    //SAStructProp.PStruct_Array level
    int columnIndex = 1;
    auto const& pstructArrayColumnInfo = topLevelStructValue.GetValue(columnIndex).GetColumnInfo();
    AssertColumnInfo("PStruct_Array", false, "SAStructProp.PStruct_Array", "SA", nullptr, pstructArrayColumnInfo);
    auto const& pstructArrayValue = topLevelStructValue.GetValue(columnIndex).GetArray();

    //out of bounds test
    ASSERT_FALSE(topLevelStructValue.GetValue(-1).GetColumnInfo().IsValid()) << "GetValue (-1).GetColumnInfo () for struct value";
    ASSERT_FALSE(topLevelStructValue.GetValue(topLevelStructValue.GetMemberCount()).GetColumnInfo().IsValid()) << "GetValue (N).GetColumnInfo with N being too large index for struct value";

    //SAStructProp.PStruct_Array[] level
    int arrayIndex = 0;
    Utf8String expectedPropPath;
    for (IECSqlValue const* arrayElement : pstructArrayValue)
        {
        IECSqlStructValue const& pstructArrayElement = arrayElement->GetStruct();
        //first struct member
        auto const& arrayElementFirstColumnInfo = pstructArrayElement.GetValue(0).GetColumnInfo();
        ASSERT_FALSE(arrayElementFirstColumnInfo.IsValid());

        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].b", arrayIndex);
        AssertColumnInfo("b", false, expectedPropPath.c_str(), "SA", nullptr, arrayElementFirstColumnInfo);

        //second struct member
        auto const& arrayElementSecondColumnInfo = pstructArrayElement.GetValue(1).GetColumnInfo();
        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].bi", arrayIndex);
        AssertColumnInfo("bi", false, expectedPropPath.c_str(), "SA", nullptr, arrayElementSecondColumnInfo);

        //out of bounds test
        ASSERT_FALSE(pstructArrayElement.GetValue(-1).GetColumnInfo().IsValid()) << "GetValue (-1).GetColumnInfo () for struct array value";
        ASSERT_FALSE(pstructArrayElement.GetValue(pstructArrayElement.GetMemberCount()).GetColumnInfo().IsValid()) << "GetValue (N).GetColumnInfo with N being too large index for struct array value";

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Step)
    {
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);
    ASSERT_TRUE(ecdb.IsDbOpen());

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT * FROM ecsql.P"));

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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ecsql.P (I, L) VALUES (100, 10203)"));

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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (?, ?)"));

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
    ASSERT_GT(secondECInstanceKey.GetECInstanceId().GetValue(), firstECInstanceKey.GetECInstanceId().GetValue());

    statement.Finalize();
    stat = statement.Prepare(ecdb, "SELECT ECInstanceId, I, S FROM ecsql.PSA WHERE ECInstanceId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    //check first insert
    stat = statement.BindId(1, firstECInstanceKey.GetECInstanceId());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    ASSERT_EQ(firstECInstanceKey.GetECInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(firstIntVal, statement.GetValueInt(1));
    ASSERT_STREQ(firstStringVal, statement.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    //check second insert
    stat = statement.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stat = statement.BindId(1, secondECInstanceKey.GetECInstanceId());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    ASSERT_EQ(secondECInstanceKey.GetECInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(secondIntVal, statement.GetValueInt(1));
    ASSERT_STREQ(secondStringVal, statement.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Reset)
    {
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    {
    ECSqlStatement stmt;
    auto stat = stmt.Prepare(ecdb, "SELECT * FROM ecsql.P LIMIT 2");
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }

    ASSERT_EQ(perClassRowCount, actualRowCount);
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";

    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }
    ASSERT_EQ(perClassRowCount, actualRowCount);

    //now finalize and do the exact same stuff. In particular this tests that the cursor is reset so that we get all results
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    {
    ECDbIssueListener issueListener(ecdb);
    ECSqlStatement stmt;
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "new ECSqlStatement";

    auto stat = stmt.Prepare(ecdb, "SELECT * FROM ecsql.P WHERE I = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Prepare.";
    }

    {
    ECDbIssueListener issueListener(ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    ECDbIssue lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.IsIssue()) << "After preparing invalid ECSQL.";
    Utf8String actualLastStatusMessage = lastIssue.GetMessage();
    ASSERT_STREQ(actualLastStatusMessage.c_str(), "Invalid ECClass expression 'blablabla'. ECClasses must always be fully qualified in ECSQL: <schema name or prefix>.<class name>");

    stmt.Finalize();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Finalize";

    //now reprepare with valid ECSQL
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Prepare";
    }
   
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetValueWithPartialPoints)
    {
    ECDbCR ecdb = SetupECDb("jsonreaderpartialpoints.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA(P2D.X,P3D.Y,PStructProp.p2d.y,PStructProp.p3d.z) VALUES(1.0, 2.0, 3.0, 4.0)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT P2D,P3D,PStructProp.p2d,PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetECInstanceId()));
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    IECSqlBinder& structBinder = statement.GetBinder(1);

    ASSERT_EQ(ECSqlStatus::Success, structBinder["Geometry"].BindGeometry(*expectedGeomSingle));

    IECSqlBinder& arrayBinder = structBinder["Geometry_Array"];
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    ecdb.SaveChanges();

    //now verify the inserts
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT B, Geometry_Array, Geometry FROM ecsql.PASpatial")) << "Preparation failed";
    int rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ASSERT_TRUE(statement.GetValueBoolean(0)) << "First column value is expected to be true";

        IECSqlArrayValue const& arrayVal = statement.GetValueArray(1);
        int i = 0;
        for (IECSqlValue const* arrayElem : arrayVal)
            {
            IGeometryPtr actualGeom = arrayElem->GetGeometry();
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT SpatialStructProp.Geometry_Array, SpatialStructProp.Geometry FROM ecsql.SSpatial")) << "Preparation failed";
    rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        IECSqlArrayValue const& arrayVal = statement.GetValueArray(0);
        int i = 0;
        for (IECSqlValue const* arrayElem : arrayVal)
            {
            IGeometryPtr actualGeom = arrayElem->GetGeometry();
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT SpatialStructProp FROM ecsql.SSpatial")) << "Preparation failed";
    rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        IECSqlStructValue const& structVal = statement.GetValueStruct(0);
        for (int i = 0; i < structVal.GetMemberCount(); i++)
            {
            IECSqlValue const& structMemberVal = structVal.GetValue(0);
            Utf8StringCR structMemberName = structMemberVal.GetColumnInfo().GetProperty()->GetName();
            if (structMemberName.Equals("Geometry"))
                {
                IGeometryPtr actualGeom = structMemberVal.GetGeometry();
                AssertGeometry(*expectedGeomSingle, *actualGeom, "SSpatial.SpatialStructProp > Geometry");
                }
            else if (structMemberName.Equals("Geometry_Array"))
                {
                IECSqlArrayValue const& arrayVal = structMemberVal.GetArray();
                int i = 0;
                for (IECSqlValue const* arrayElem : arrayVal)
                    {
                    IGeometryPtr actualGeom = arrayElem->GetGeometry();
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
    ECDbCR ecdb = SetupECDb("ecsql_geometry.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <SharedColumnCount>2</SharedColumnCount>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECArrayProperty propertyName="GeomArray" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Geom_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECArrayProperty propertyName="GeomArray_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    std::vector<IGeometryPtr> expectedGeoms {IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0))),
        IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 1.0, 1.0, 2.0, 2.0, 2.0))),
        IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(2.0, 2.0, 2.0, 3.0, 3.0, 3.0)))};

    IGeometryPtr expectedGeomSingle = expectedGeoms[0];

    ECInstanceKey key;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.Element(Geom,GeomArray,Geom_Overflow,GeomArray_Overflow) VALUES(?,?,?,?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT Geom,GeomArray,Geom_Overflow,GeomArray_Overflow FROM ts.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << statement.GetECSql();

    AssertGeometry(*expectedGeomSingle, *statement.GetValueGeometry(0), "Geometry property");
    size_t arrayIndex = 0;
    for (IECSqlValue const* arrayElementVal : statement.GetValueArray(1))
        {
        AssertGeometry(*expectedGeoms[arrayIndex], *arrayElementVal->GetGeometry(), "Geometry array property");
        arrayIndex++;
        }
    AssertGeometry(*expectedGeomSingle, *statement.GetValueGeometry(2), "Geometry property overflow");
    arrayIndex = 0;
    for (IECSqlValue const* arrayElementVal : statement.GetValueArray(3))
        {
        AssertGeometry(*expectedGeoms[arrayIndex], *arrayElementVal->GetGeometry(), "Geometry array property overflow");
        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetGeometryWithInvalidBlobFormat)
    {
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    // insert invalid geom blob
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "INSERT INTO ecsqltest_PASpatial (ECInstanceId, Geometry) VALUES (1,?)"));
    double dummyValue = 3.141516;
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindBlob(1, &dummyValue, (int) sizeof(dummyValue), Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(ecdb, "SELECT Geometry FROM ecsql.PASpatial"));
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
    const auto perClassRowCount = 0;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp) VALUES(?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

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

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "Step for '" << ecsql << "' failed";
    statement.Finalize();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto &pStructArray = stmt.GetValue(0).GetArray();
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayInsertWithParametersLongAndArray)
    {
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(123, ?)";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

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
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ecsql.PSA");
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto ecsql = "INSERT INTO ecsql.Sub1 (I,Sub1I) VALUES(123, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt(1, 333);

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ecsql.PSA");
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto ecsql = "INSERT INTO ecsql.P (B,D,I,L,S) VALUES(1, ?,?,123,?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindDouble(1, 2.22);
    statement.BindInt(2, 123);
    statement.BindText(3, "Test Test", IECSqlBinder::MakeCopy::Yes);
    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ecsql.PSA");
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, ecsql);
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
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto &pStructArray = stmt.GetValue(0).GetArray();
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        //need to Verify all values
        /*  for (auto const & arrayItem : pStructArray)
        {
        IECSqlStructValue const & structValue = arrayItem->GetStruct();
        IECSqlValue const & value= structValue.GetValue(0);
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
* @bsiclass                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, StructUpdateWithDotOperator)
    {
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp.i) VALUES(2)";

    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    {
    auto prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ecsql.SA");
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
    stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();

    auto prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ecsql.SA");
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
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement insertStatement;
    auto stat = insertStatement.Prepare(ecdb, ecsql);
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
    auto prepareStatus = selectStatement.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (selectStatement.Step() == BE_SQLITE_ROW)
        {
        auto &pStructArray = selectStatement.GetValue(0).GetArray();
        ASSERT_EQ(count, pStructArray.GetArrayLength());
        }

    ECSqlStatement updateStatement;
    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStruct_Array=?";
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.Prepare(ecdb, ecsql)) << ecsql;
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
    prepareStatus = statement.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto &pStructArray = statement.GetValue(0).GetArray();
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

    SetupECDb("AmbiguousQuery.ecdb", schemaXml);

    ECN::ECSchemaCP schema = GetECDb().Schemas().GetECSchema("TestSchema");

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
    ECInstanceInserter inserter(GetECDb(), *TestClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*Instance1, true));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*Instance2, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT P1 FROM ts.TestClass"));
    Utf8String ExpectedValueOfP1 = "Harvey-Mike-";
    Utf8String ActualValueOfP1;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ActualValueOfP1.append(stmt.GetValueText(0));
        ActualValueOfP1.append("-");
        }

    ASSERT_EQ(ExpectedValueOfP1, ActualValueOfP1);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT TestClass.TestClass.P1 FROM ts.TestClass"));
    Utf8String ExpectedValueOfStructP1 = "val1-val2-";
    Utf8String ActualValueOfStructP1;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ActualValueOfStructP1.append(stmt.GetValueText(0));
        ActualValueOfStructP1.append("-");
        }

    ASSERT_EQ(ExpectedValueOfStructP1, ActualValueOfStructP1);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT TestClass.P2 FROM ts.TestClass"));
    int ActualValueOfStructP2 = 468;
    int ExpectedValueOfStructP2 = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ExpectedValueOfStructP2 += stmt.GetValueInt(0);
        }

    ASSERT_EQ(ExpectedValueOfStructP2, ActualValueOfStructP2);
    stmt.Finalize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Umer Sufyan                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, AmbiguousJoin)
    {
    ECDbCR ecdb = SetupECDb("ambiguousjoin.ecdb", BeFileName(L"Computers.01.00.ecschema.xml"));
    ECSqlStatement stmt;

    auto stat = stmt.Prepare(ecdb, "SELECT * FROM TR.Laptop JOIN TR.Laptop USING TR.LaptopHasLaptop FORWARD");

    ASSERT_NE(ECSqlStatus::Success, stat);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                 10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindNegECInstanceId)
    {
    ECDbR ecdb = SetupECDb("BindNegECInstanceId.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    ECSqlStatement stmt;

    //Inserting Values for a negative ECInstanceId.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId,B,D,S) VALUES(?,?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, (int64_t) (-1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(2, true));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 100.54));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, "Foo", IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Retrieving Values.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "Select B,D,S FROM ecsql.P WHERE ECInstanceId=-1"));
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

    SetupECDb("InstanceInsertionInArray.ecdb", schema);

    std::vector<Utf8String> expectedStringArray = {"val1", "val2", "val3"};
    std::vector<int> expectedIntArray = {200, 400, 600};

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.TestClass (P1, P2_Array, P3_Array) VALUES(?, ?, ?)"));

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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT P1, P2_Array, P3_Array FROM ts.TestClass"));
    while (stmt.Step() == DbResult::BE_SQLITE_ROW)
        {
        ASSERT_STREQ("Foo", stmt.GetValueText(0));

        IECSqlArrayValue const& StringArray = stmt.GetValueArray(1);
        size_t expectedIndex = 0;

        for (IECSqlValue const* arrayElement : StringArray)
            {
            Utf8CP actualArrayElement = arrayElement->GetText();
            ASSERT_STREQ(expectedStringArray[expectedIndex].c_str(), actualArrayElement);
            expectedIndex++;
            }

        IECSqlArrayValue const& IntArray = stmt.GetValueArray(2);
        expectedIndex = 0;
        for (IECSqlValue const* arrayElement : IntArray)
            {
            int actualArrayElement = arrayElement->GetInt();
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
        ECN::ECSchemaCP existing = db.Schemas().GetECSchema(schemaIn.GetName().c_str());
        if (nullptr != existing)
            return;

        ECN::ECSchemaPtr imported = nullptr;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, schemaIn.CopySchema(imported));

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetName(schemaIn.GetName()));

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetAlias(schemaIn.GetAlias()));

        ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
        ASSERT_EQ(ECN::ECObjectsStatus::Success, contextPtr->AddSchema(*imported));

        ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(contextPtr->GetCache().GetSchemas()));
        };

    ECDbR ecdb = SetupECDb("ImportTwoInARow.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    {
    ECN::ECSchemaPtr schema;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ECN::ECSchema::CreateSchema(schema, "ImportTwoInARow", "tir", 0, 0, 0));

    ECN::ECEntityClassP ecclass;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, schema->CreateEntityClass(ecclass, "C1"));

    ECN::PrimitiveECPropertyP ecprop;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecclass->CreatePrimitiveProperty(ecprop, "X"));

    ecprop->SetType(ECN::PRIMITIVETYPE_Double);

    importSchema(ecdb, *schema);
    }

    EC::ECSqlStatement selectC1;
    selectC1.Prepare(ecdb, "SELECT ECInstanceId FROM tir.C1");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                     06/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PointsMappedToSharedColumns)
    {
    ECDbCR ecdb = SetupECDb("pointsmappedtosharedcolumns.ecdb", SchemaItem(
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
        "              <SharedColumnCount>4</SharedColumnCount>"
        "           </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Prop2' typeName='double' />"
        "        <ECProperty propertyName='Center' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Sub1(Prop1,Center) VALUES(1.1,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(1, DPoint3d::From(1.0, 2.0, 3.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECClassId sub1ClassId = GetECDb().Schemas().GetECSchema("TestSchema")->GetClassCP("Sub1")->GetId();
    Utf8String expectedNativeSql;
    expectedNativeSql.Sprintf("INSERT INTO [ts_Base] ([Prop1],[ECInstanceId],ECClassId) VALUES (1.1,:_ecdbparam1,%s);INSERT INTO [ts_Sub1] ([sc2],[sc3],[sc4],[BaseECInstanceId],ECClassId) VALUES (:_ecdbparam1,:_ecdbparam2,:_ecdbparam3,:_ecdbecinstanceid_1,%s)", sub1ClassId.ToString().c_str(), sub1ClassId.ToString().c_str());
    ASSERT_STREQ(expectedNativeSql.c_str(), stmt.GetNativeSql());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Center.X, Center.Y, Center.Z FROM ts.Sub1 LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1.0, stmt.GetValueDouble(0));
    ASSERT_EQ(2.0, stmt.GetValueDouble(1));
    ASSERT_EQ(3.0, stmt.GetValueDouble(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.Sub1 WHERE Center.X > 0 AND Center.Y > 0 AND Center.Z > 0"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindZeroBlob)
    {
    SetupECDb("bindzeroblob.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <SharedColumnCount>5</SharedColumnCount>
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
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Element(Prop1,Prop2,Prop3,Prop4,Prop5,Prop1_Overflow,Prop2_Overflow) VALUES(?,?,?,?,?,?,?)"));
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
        ECClassCP ecClass = ecdb.Schemas().GetECClass("ECSqlTest", className);
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
        ASSERT_EQ(SUCCESS, ecdb.OpenBlobIO(io, *ecClass, accessString, key.GetECInstanceId(), true));
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
                      ecClass->GetECSqlName().c_str(), key.GetECInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        int actualBlobSize = -1;
        ASSERT_STREQ("Hello, world", (Utf8CP) stmt.GetValueBlob(0, &actualBlobSize)) << stmt.GetECSql();
        ASSERT_EQ(expectedBlobSize, actualBlobSize) << stmt.GetECSql();

        //validate result via blobio
        ASSERT_EQ(SUCCESS, ecdb.OpenBlobIO(io, *ecClass, accessString, key.GetECInstanceId(), false));
        ASSERT_TRUE(io.IsValid());
        ASSERT_EQ(expectedBlobSize, io.GetNumBytes());
        Utf8String actualBlobBuffer;
        actualBlobBuffer.reserve((size_t) expectedBlobSize);
        ASSERT_EQ(BE_SQLITE_OK, io.Read(const_cast<Utf8P> (actualBlobBuffer.data()), expectedBlobSize, 0));
        ASSERT_STREQ("Hello, world", actualBlobBuffer.c_str()) << "BlobIO::Read";
        };


    ECDbCR ecdb = SetupECDb("blobio.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    assertBlobIO(ecdb, "P", "Bi");
    assertBlobIO(ecdb, "PSA", "PStructProp.bi");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BlobIOForInvalidProperties)
    {
    SetupECDb("blobioinvalidcases.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    {
    ECClassCP ecClass = GetECDb().Schemas().GetECClass("ECSqlTest", "PSA");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        const bool expectedToSucceed = prop->GetIsPrimitive() && (prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_Binary || prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_IGeometry);
        if (!expectedToSucceed)
            {
            BlobIO io;
            ASSERT_EQ(ERROR, GetECDb().OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Not a binary/geometry property";
            ASSERT_FALSE(io.IsValid());
            }
        }
    }

    {
    ECClassCP ecClass = GetECDb().Schemas().GetECClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        BlobIO io;
        ASSERT_EQ(ERROR, GetECDb().OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Cannot use BlobIO on ECStructs";
        ASSERT_FALSE(io.IsValid());
        }
    }

    {
    ECClassCP ecClass = GetECDb().Schemas().GetECClass("ECDbMap", "ClassMap");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties())
        {
        BlobIO io;
        ASSERT_EQ(ERROR, GetECDb().OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Cannot use BlobIO on custom attribute classes";
        ASSERT_FALSE(io.IsValid());
        }
    }

    GetECDb().CloseDb();

    {
    SetupECDb("blobioinvalidcases2.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <SharedColumnCount>2</SharedColumnCount>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Prop1" typeName="Binary" />
                                <ECProperty propertyName="Prop2" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Prop1_Overflow" typeName="Binary" />
                                <ECProperty propertyName="Prop2_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey key;
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO TestSchema.Element(Prop1,Prop2,Prop1_Overflow,Prop2_Overflow) VALUES(zeroblob(10),?,?,?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, 10)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(2, 10)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(3, 10)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        }

    ECClassCP testClass = GetECDb().Schemas().GetECClass("TestSchema", "Element");
    ASSERT_TRUE(testClass != nullptr);

        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, GetECDb().OpenBlobIO(io, *testClass, "Prop1", key.GetECInstanceId(), true)) << "Binary property not mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, GetECDb().OpenBlobIO(io, *testClass, "Prop2", key.GetECInstanceId(), true)) << "IGeometry property not mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, GetECDb().OpenBlobIO(io, *testClass, "Prop1_Overflow", key.GetECInstanceId(), true)) << "Binary property mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, GetECDb().OpenBlobIO(io, *testClass, "Prop2_Overflow", key.GetECInstanceId(), true)) << "IGeometry property mapped to overflow table";
        }
    }
    }

END_ECDBUNITTESTS_NAMESPACE
