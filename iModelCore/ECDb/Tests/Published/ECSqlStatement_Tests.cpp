/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatement_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"
#include "NestedStructArrayTestSchemaHelper.h"
#include <cmath>
#include <algorithm>

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

        virtual void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "Select POW(ECInstanceId, 2), GetECClassId() ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW(ECInstanceId, 2), GetECClassId() ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*), Phone FROM (SELECT GetECClassId() ECClassId, Phone FROM ECST.Supplier UNION ALL SELECT GetECClassId() ECClassId, Phone FROM ECST.Customer) GROUP BY ECClassId ORDER BY Phone"));

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
    ECClassId supplierClassId = ecdb.Schemas().GetECClassId("ECST", "Supplier", ResolveSchema::BySchemaNamespacePrefix);
    ECClassId customerClassId = ecdb.Schemas().GetECClassId("ECST", "Customer", ResolveSchema::BySchemaNamespacePrefix);
    ECClassId firstClassId = std::min<ECClassId>(supplierClassId, customerClassId);
    ECClassId secondClassId = std::max<ECClassId>(supplierClassId, customerClassId);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, COUNT(*) FROM (SELECT GetECClassId() ECClassId FROM ECST.Supplier UNION ALL SELECT GetECClassId() ECClassId FROM ECST.Customer) GROUP BY ECClassId ORDER BY ECClassId"));
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

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+1+GetECClassId(), true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000/5, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+2+GetECClassId(), false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000+100*5, true, 'LCD')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductName FROM ECST.Product WHERE UnitPrice=100+2+GetECClassId()"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Table", statement.GetValueText(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice>GetECClassId() AND ProductName='Chair'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+GetECClassId() AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice=100+3+GetECClassId() OR ProductName='LCD'"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, GetECClassId(), I,T FROM nsat.ClassA ORDER BY ECInstanceId"));
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
        ECInstanceInserter inserter(ecdb, inst->GetClass());
        ASSERT_TRUE(inserter.IsValid());
        ASSERT_EQ(BentleyStatus::SUCCESS, inserter.Insert(*inst, true));
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
        ECInstanceInserter inserter(ecdb, inst->GetClass());
        auto st = inserter.Insert(*inst);
        ASSERT_TRUE(st == BentleyStatus::SUCCESS);
        insertCount++;
        }

    ECClassCP classP = ecdb.Schemas().GetECClass("NestedStructArrayTest", "ClassP");
    ASSERT_TRUE(classP != nullptr);

    ECInstanceDeleter deleter(ecdb, *classP);

    int deleteCount = 0;
    for (auto& inst : in)
        {
        ASSERT_TRUE(deleter.Delete(*inst) == BentleyStatus::SUCCESS);
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
// @bsiclass                                     Krischan.Eberle                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ECInstanceIdColumnInfo)
    {
    const int perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount, ECDb::OpenParams(Db::OpenMode::Readonly));

    auto ecsql = "SELECT c1.ECInstanceId, c2.ECInstanceId FROM ecsql.PSA c1, ecsql.P c2 LIMIT 1";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    auto const& value1 = statement.GetValue(0);
    auto const& columnInfo1 = value1.GetColumnInfo();

    ASSERT_FALSE(value1.IsNull());
    ASSERT_FALSE(columnInfo1.IsGeneratedProperty());
    ASSERT_STREQ("ECSqlSystemProperties", columnInfo1.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c1", columnInfo1.GetRootClassAlias());
    ASSERT_STREQ("PSA", columnInfo1.GetRootClass().GetName().c_str());

    auto const& value2 = statement.GetValue(1);
    auto const& columnInfo2 = value2.GetColumnInfo();

    ASSERT_FALSE(value2.IsNull());
    ASSERT_FALSE(columnInfo2.IsGeneratedProperty());
    ASSERT_STREQ("ECSqlSystemProperties", columnInfo2.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c2", columnInfo2.GetRootClassAlias());
    ASSERT_STREQ("P", columnInfo2.GetRootClass().GetName().c_str());
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

    ECDbIssueListener issueListener(ecdb);
    auto& arrayBinder = statement.BindArray(2, (uint32_t) count);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "BindArray failed";
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "AddArrayElement failed";
        auto stat = structBinder.GetMember("d").BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("i").BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("l").BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
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

    ECDbIssueListener issueListener(ecdb);

    //add three array elements
    const uint32_t arraySize = 3;
    auto& arrayBinder = statement.BindArray(2, arraySize);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "BindArray failed";
    for (int i = 0; i < arraySize; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "AddArrayElement failed";
        auto stat = structBinder.GetMember("d").BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("i").BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("l").BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
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
    auto stat = statement.Prepare(ecdb, "INSERT INTO ecsql.PA (I_Array,S_Array) VALUES(:ia,:sa)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    auto& arrayBinderI = statement.BindArray(1, (int) expectedIntArray.size());
    for (int arrayElement : expectedIntArray)
        {
        auto& elementBinder = arrayBinderI.AddArrayElement();
        elementBinder.BindInt(arrayElement);
        }

    auto& arrayBinderS = statement.BindArray(2, (int) expectedStringArray.size());
    for (Utf8StringCR arrayElement : expectedStringArray)
        {
        auto& elementBinder = arrayBinderS.AddArrayElement();
        elementBinder.BindText(arrayElement.c_str(), IECSqlBinder::MakeCopy::No);
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
        ECDbIssueListener issueListener(ecdb);
        int actualArrayElement = arrayElement->GetInt();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
        ASSERT_EQ(expectedIntArray[expectedIndex], actualArrayElement);
        expectedIndex++;
        }

    IECSqlArrayValue const& stringArray = statement.GetValueArray(1);
    expectedIndex = 0;
    for (IECSqlValue const* arrayElement : stringArray)
        {
        ECDbIssueListener issueListener(ecdb);
        auto actualArrayElement = arrayElement->GetText();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
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


    ECDbIssueListener issueListener(ecdb);
    auto& arrayBinderDt = statement.BindArray(1, 3);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    for (DateTimeCR testDate : testDates)
        {
        auto& elementBinder = arrayBinderDt.AddArrayElement();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

        ECSqlStatus expectedStat = testDate.GetInfo().GetKind() == DateTime::Kind::Local ? ECSqlStatus::Error : ECSqlStatus::Success;
        ASSERT_EQ(expectedStat, elementBinder.BindDateTime(testDate));
        issueListener.Reset();
        }


    auto& arrayBinderDtUtc = statement.BindArray(2, 3);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    for (DateTimeCR testDate : testDates)
        {
        auto& elementBinder = arrayBinderDtUtc.AddArrayElement();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

        ECSqlStatus expectedStat = testDate.GetInfo().GetKind() == DateTime::Kind::Utc ? ECSqlStatus::Success : ECSqlStatus::Error;
        ASSERT_EQ(expectedStat, elementBinder.BindDateTime(testDate));
        issueListener.Reset();
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

        auto& arrayBinder = statement.BindArray(1, 5);
        for (int i = 0; i < count; i++)
            {
            auto& elementBinder = arrayBinder.AddArrayElement();
            elementBinder.BindInt(i);
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

        IECSqlArrayBinder& arrayBinder = statement.BindArray(1, 5);
        for (int i = 0; i < count; i++)
            {
            IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
            elementBinder.BindStruct().GetMember("i").BindInt(i);
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
    const auto perClassRowCount = 10;
    ECDbR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    auto testFunction = [this, &ecdb] (Utf8CP insertECSql, bool bindExpectedToSucceed, int structParameterIndex, Utf8CP structValueJson, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue(Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse(structValueJson, expectedStructValue);
        ASSERT_TRUE(parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare(ecdb, insertECSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of ECSQL '" << insertECSql << "' failed";

        ECDbIssueListener issueListener(ecdb);
        auto& binder = statement.GetBinder(structParameterIndex);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

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

        issueListener.Reset();
        IECSqlValue const& structValue = statement.GetValue(structValueIndex);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
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

        ECDbIssueListener issueListener(ecdb);
        IECSqlBinder& structBinder = statement.GetBinder(structParameterIndex);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

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

        ECDbIssueListener issueListener(ecdb);
        auto& binder = statement.GetBinder(structParameterIndex);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

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

        issueListener.Reset();
        IECSqlValue const& structValue = statement.GetValue(structValueIndex);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
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
    IECSqlStructBinder& binder = stmt.BindStruct(1);
    binder.GetMember("First").BindText("John", IECSqlBinder::MakeCopy::No);
    binder.GetMember("Last").BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.Person WHERE FullName<>?"));
    IECSqlStructBinder& binder = stmt.BindStruct(1);
    binder.GetMember("First").BindText("John", IECSqlBinder::MakeCopy::No);
    binder.GetMember("Last").BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetNativeSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT count(*) FROM ts.Person WHERE FullName IN (?,?,?)"));

    IECSqlStructBinder& binder1 = stmt.BindStruct(1);
    binder1.GetMember("First").BindText("John", IECSqlBinder::MakeCopy::No);
    binder1.GetMember("Last").BindText("Myer", IECSqlBinder::MakeCopy::No);

    IECSqlStructBinder& binder2 = stmt.BindStruct(2);
    binder2.GetMember("First").BindText("Rich", IECSqlBinder::MakeCopy::No);
    binder2.GetMember("Last").BindText("Myer", IECSqlBinder::MakeCopy::No);

    IECSqlStructBinder& binder3 = stmt.BindStruct(3);
    binder3.GetMember("First").BindText("John", IECSqlBinder::MakeCopy::No);
    binder3.GetMember("Last").BindText("Smith", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "UPDATE ts.Person SET FullName.[Last]='Meyer' WHERE FullName=?"));

    IECSqlStructBinder& binder = stmt.BindStruct(1);
    binder.GetMember("First").BindText("John", IECSqlBinder::MakeCopy::No);
    binder.GetMember("Last").BindText("Myer", IECSqlBinder::MakeCopy::No);
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
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=True"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=False"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=0"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=1"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ECClassId=")) << "Native SQL: " << nativeSql.c_str();
    }

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
    ECDbIssueListener issueListener(ecdb);
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("Dt_Array", false, "Dt_Array", "PSA", "c", topLevelColumnInfo);
    issueListener.Reset();
    auto const& topLevelArrayValue = stmt.GetValueArray(0);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    //out of bounds test
    BeTest::SetFailOnAssert(false);
    {
    stmt.GetColumnInfo(-1);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::GetColumnInfo (-1) is expected to fail";
    stmt.GetColumnInfo(2);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::GetColumnInfo is expected to fail with too large index";
    }
    BeTest::SetFailOnAssert(true);

    //In array level
    int arrayIndex = 0;
    for (IECSqlValue const* arrayElement : topLevelArrayValue)
        {
        issueListener.Reset();
        auto const& arrayElementColumnInfo = arrayElement->GetColumnInfo();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "Primitive array element IECSqlValue::GetColumnInfo failed.";
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

    ECDbIssueListener issueListener(ecdb);
    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("SAStructProp", false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);

    //out of bounds test
    BeTest::SetFailOnAssert(false);
    {
    issueListener.Reset();
    stmt.GetColumnInfo(-1);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::GetColumnInfo (-1)";
    stmt.GetColumnInfo(2);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::GetColumnInfo with too large index";
    }
    BeTest::SetFailOnAssert(true);

    //SAStructProp.PStructProp level
    auto const& topLevelStructValue = stmt.GetValueStruct(0);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    auto const& nestedStructPropColumnInfo = topLevelStructValue.GetValue(0).GetColumnInfo(); //0 refers to first member in SAStructProp which is PStructProp
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "Struct IECSqlValue::GetColumnInfo ()";
    AssertColumnInfo("PStructProp", false, "SAStructProp.PStructProp", "SA", nullptr, nestedStructPropColumnInfo);
    auto const& nestedStructValue = topLevelStructValue.GetValue(0).GetStruct();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "Struct IECSqlValue::GetStruct ()";;

    //out of bounds test
    BeTest::SetFailOnAssert(false);
    {
    topLevelStructValue.GetValue(-1);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue (-1) for struct value.";
    topLevelStructValue.GetValue(topLevelStructValue.GetMemberCount());
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue with too large index for struct value";
    }
    BeTest::SetFailOnAssert(true);

    //SAStructProp.PStructProp.XXX level
    auto const& firstStructMemberColumnInfo = nestedStructValue.GetValue(0).GetColumnInfo();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("b", false, "SAStructProp.PStructProp.b", "SA", nullptr, firstStructMemberColumnInfo);

    issueListener.Reset();
    auto const& secondStructMemberColumnInfo = nestedStructValue.GetValue(1).GetColumnInfo();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("bi", false, "SAStructProp.PStructProp.bi", "SA", nullptr, secondStructMemberColumnInfo);

    issueListener.Reset();
    auto const& eighthStructMemberColumnInfo = nestedStructValue.GetValue(8).GetColumnInfo();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("p2d", false, "SAStructProp.PStructProp.p2d", "SA", nullptr, eighthStructMemberColumnInfo);

    //out of bounds test
    BeTest::SetFailOnAssert(false);
    {
    issueListener.Reset();
    nestedStructValue.GetValue(-1).GetColumnInfo();
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue (-1) for struct value on second nesting level.";
    nestedStructValue.GetValue(nestedStructValue.GetMemberCount());
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue with too large index for struct value on second nesting level.";
    }
    BeTest::SetFailOnAssert(true);
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

    ECDbIssueListener issueListener(ecdb);
    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("SAStructProp", false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);
    issueListener.Reset();
    auto const& topLevelStructValue = stmt.GetValueStruct(0);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    //out of bounds test
    BeTest::SetFailOnAssert(false);
    {
    stmt.GetColumnInfo(-1);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::GetColumnInfo (-1)";
    stmt.GetColumnInfo(2);
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::GetColumnInfo with too large index";
    }
    BeTest::SetFailOnAssert(true);

    //SAStructProp.PStruct_Array level
    int columnIndex = 1;
    auto const& pstructArrayColumnInfo = topLevelStructValue.GetValue(columnIndex).GetColumnInfo();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());
    AssertColumnInfo("PStruct_Array", false, "SAStructProp.PStruct_Array", "SA", nullptr, pstructArrayColumnInfo);
    issueListener.Reset();
    auto const& pstructArrayValue = topLevelStructValue.GetValue(columnIndex).GetArray();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    //out of bounds test
    BeTest::SetFailOnAssert(false);
    {
    topLevelStructValue.GetValue(-1).GetColumnInfo();
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue (-1).GetColumnInfo () for struct value";
    topLevelStructValue.GetValue(topLevelStructValue.GetMemberCount()).GetColumnInfo();
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue (N).GetColumnInfo with N being too large index for struct value";
    }
    BeTest::SetFailOnAssert(true);

    //SAStructProp.PStruct_Array[] level
    int arrayIndex = 0;
    Utf8String expectedPropPath;
    for (IECSqlValue const* arrayElement : pstructArrayValue)
        {
        IECSqlStructValue const& pstructArrayElement = arrayElement->GetStruct();
        //first struct member
        issueListener.Reset();
        auto const& arrayElementFirstColumnInfo = pstructArrayElement.GetValue(0).GetColumnInfo();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].b", arrayIndex);
        AssertColumnInfo("b", false, expectedPropPath.c_str(), "SA", nullptr, arrayElementFirstColumnInfo);

        //second struct member
        issueListener.Reset();
        auto const& arrayElementSecondColumnInfo = pstructArrayElement.GetValue(1).GetColumnInfo();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].bi", arrayIndex);
        AssertColumnInfo("bi", false, expectedPropPath.c_str(), "SA", nullptr, arrayElementSecondColumnInfo);

        //out of bounds test
        BeTest::SetFailOnAssert(false);
        {
        issueListener.Reset();
        pstructArrayElement.GetValue(-1).GetColumnInfo();
        ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue (-1).GetColumnInfo () for struct array value";
        pstructArrayElement.GetValue(pstructArrayElement.GetMemberCount()).GetColumnInfo();
        ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "GetValue (N).GetColumnInfo with N being too large index for struct array value";
        }
        BeTest::SetFailOnAssert(true);

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

    issueListener.Reset();
    auto stat = stmt.Prepare(ecdb, "SELECT * FROM ecsql.P WHERE I = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Prepare.";

    issueListener.Reset();
    BeTest::SetFailOnAssert(false);
    stat = stmt.BindPoint2D(1, DPoint2d::From(1.0, 1.0));
    ASSERT_EQ(ECSqlStatus::Error, stat) << "Cannot bind points to int parameter";
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "Cannot bind points to int parameter";
    BeTest::SetFailOnAssert(true);

    issueListener.Reset();
    stat = stmt.BindInt(1, 123);
    ASSERT_EQ(ECSqlStatus::Success, stat);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    issueListener.Reset();
    BeTest::SetFailOnAssert(false);
    stat = stmt.BindDouble(2, 3.14);
    BeTest::SetFailOnAssert(true);
    ASSERT_EQ(ECSqlStatus::Error, stat) << "Index out of bounds error expected.";
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "Index out of bounds error expected.";

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Step";

        stmt.GetColumnInfo(0);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

        BeTest::SetFailOnAssert(false);
        stmt.GetColumnInfo(-1);
        BeTest::SetFailOnAssert(true);
        ASSERT_TRUE(issueListener.GetIssue().IsIssue());
        stmt.GetColumnInfo(0);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
        BeTest::SetFailOnAssert(false);
        stmt.GetColumnInfo(100);
        BeTest::SetFailOnAssert(true);
        ASSERT_TRUE(issueListener.GetIssue().IsIssue());
        stmt.GetColumnInfo(0);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());

        BeTest::SetFailOnAssert(false);
        stmt.GetValueInt(-1);
        BeTest::SetFailOnAssert(true);
        ASSERT_TRUE(issueListener.GetIssue().IsIssue());
        stmt.GetValueInt(0);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
        BeTest::SetFailOnAssert(false);
        stmt.GetValueInt(100);
        BeTest::SetFailOnAssert(true);
        ASSERT_TRUE(issueListener.GetIssue().IsIssue());
        stmt.GetValueDouble(1);
        ASSERT_FALSE(issueListener.GetIssue().IsIssue());
        BeTest::SetFailOnAssert(false);
        stmt.GetValuePoint2D(1);
        BeTest::SetFailOnAssert(true);
        ASSERT_TRUE(issueListener.GetIssue().IsIssue());
        }

    ASSERT_FALSE(issueListener.GetIssue().IsIssue());

    stat = stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stat) << "ECSqlStatement::Reset failed unexpectedly.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to ClearBindings";

    stat = stmt.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat) << "ECSqlStatement::Reset failed unexpectedly.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Reset";

    stmt.Finalize();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Finalize";
    }

    {
    ECDbIssueListener issueListener(ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    ECDbIssue lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.IsIssue()) << "After preparing invalid ECSQL.";
    Utf8String actualLastStatusMessage = lastIssue.GetMessage();
    ASSERT_STREQ(actualLastStatusMessage.c_str(), "ECClass 'blablabla' does not exist. Try using fully qualified class name: <schema name>.<class name>.");

    stmt.Finalize();
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Finalize";

    //now reprepare with valid ECSQL
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Prepare";

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Step";
        }

    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "After successful call to Step";
    }

    {
    ECDbIssueListener issueListener(ecdb);

    BeBriefcaseBasedId id(BeBriefcaseId(111), 111); //an id not used in the current file
    ECSqlStatement stmt;
    ECSqlStatus stat = stmt.Prepare(ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES (?)");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation failed unexpectedly";
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));

    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Step failed unexpectedly";
    ASSERT_EQ(id.GetValue(), newKey.GetECInstanceId().GetValue());
    stmt.Reset();
    stmt.ClearBindings();

    //reuse same id again to provoke constraint violation
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_PRIMARYKEY, stmt.Step(newKey)) << "Step succeeded unexpectedly although it should not because a row with the same ECInstanceId already exists.";
    ASSERT_TRUE(issueListener.GetIssue().IsIssue()) << "After insert of row with same ECInstanceId";
    }
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

    IECSqlArrayBinder& arrayBinder = statement.BindArray(2, 3);
    for (auto& geom : expectedGeoms)
        {
        ECDbIssueListener issueListener(ecdb);
        auto& arrayElementBinder = arrayBinder.AddArrayElement();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "IECSqlArrayBinder::AddArrayElement is expected to succeed";
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder.BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    {
    auto ecsql = "INSERT INTO ecsql.SSpatial (SpatialStructProp.Geometry, SpatialStructProp.Geometry_Array) VALUES(?,?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));

    ECDbIssueListener issueListener(ecdb);
    IECSqlArrayBinder& arrayBinder = statement.BindArray(2, 3);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::BindArray is expected to succeed";
    for (auto& geom : expectedGeoms)
        {
        issueListener.Reset();
        auto& arrayElementBinder = arrayBinder.AddArrayElement();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "IECSqlArrayBinder::AddArrayElement is expected to succeed";
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder.BindGeometry(*geom));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    auto ecsql = "INSERT INTO ecsql.SSpatial (SpatialStructProp) VALUES(?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    ECDbIssueListener issueListener(ecdb);
    IECSqlStructBinder& structBinder = statement.BindStruct(1);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "ECSqlStatement::BindStruct is expected to succeed";

    ASSERT_EQ(ECSqlStatus::Success, structBinder.GetMember("Geometry").BindGeometry(*expectedGeomSingle));

    issueListener.Reset();
    IECSqlArrayBinder& arrayBinder = structBinder.GetMember("Geometry_Array").BindArray(3);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "IECSqlBinder::BindArray is expected to succeed";
    for (auto& geom : expectedGeoms)
        {
        issueListener.Reset();
        auto& arrayElementBinder = arrayBinder.AddArrayElement();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "IECSqlArrayBinder::AddArrayElement is expected to succeed";
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder.BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    ecdb.SaveChanges();
    ecdb.ClearECDbCache();

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

        ECDbIssueListener issueListener(ecdb);
        ASSERT_TRUE(ecsqlStmt.GetValueGeometry(0) == nullptr) << "Invalid geom blob format expected so that nullptr is returned.";
        ASSERT_TRUE(issueListener.GetIssue().IsIssue());
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

    ECDbIssueListener issueListener(ecdb);
    IECSqlStructBinder& saStructBinder = statement.BindStruct(1); //SAStructProp
    ECDbIssue lastIssue = issueListener.GetIssue();
    ASSERT_FALSE(lastIssue.IsIssue()) << "AddArrayElement failed: " << lastIssue.GetMessage();
    IECSqlStructBinder& pStructBinder = saStructBinder.GetMember("PStructProp").BindStruct();
    stat = pStructBinder.GetMember("i").BindInt(99);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";

    //add three array elements
    const int count = 3;
    issueListener.Reset();
    auto& arrayBinder = saStructBinder.GetMember("PStruct_Array").BindArray((uint32_t) count);
    lastIssue = issueListener.GetIssue();
    ASSERT_FALSE(lastIssue.IsIssue()) << "BindArray failed: " << lastIssue.GetMessage();
    for (int i = 0; i < count; i++)
        {
        issueListener.Reset();
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        lastIssue = issueListener.GetIssue();
        ASSERT_FALSE(lastIssue.IsIssue()) << "AddArrayElement failed: " << lastIssue.GetMessage();
        ASSERT_EQ(ECSqlStatus::Success, structBinder.GetMember("d").BindDouble(i * PI));
        ASSERT_EQ(ECSqlStatus::Success, structBinder.GetMember("i").BindInt(i * 2));
        ASSERT_EQ(ECSqlStatus::Success, structBinder.GetMember("l").BindInt64(i * 3));
        ASSERT_EQ(ECSqlStatus::Success, structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1)));
        ASSERT_EQ(ECSqlStatus::Success, structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2)));
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

    ECDbIssueListener issueListener(ecdb);

    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray(1, (uint32_t) count);
    ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "BindArray failed";
    for (int i = 0; i < count; i++)
        {
        issueListener.Reset();
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_FALSE(issueListener.GetIssue().IsIssue()) << "AddArrayElement failed";
        auto stat = structBinder.GetMember("d").BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("i").BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("l").BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
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
    auto& arrayBinder = statement.BindArray(1, (uint32_t) count);
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        stat = structBinder.GetMember("d").BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("i").BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("l").BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
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
    auto& arrayBinder = insertStatement.BindArray(1, (uint32_t) count);
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        stat = structBinder.GetMember("d").BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("i").BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("l").BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
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
    auto& updateArrayBinder = updateStatement.BindArray(1, (uint32_t) count);
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = updateArrayBinder.AddArrayElement().BindStruct();
        stat = structBinder.GetMember("d").BindDouble(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("i").BindInt(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("l").BindInt64(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = structBinder.GetMember("p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
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
                         "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
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
    ECInstanceInserter inserter(GetECDb(), *TestClass);
    ASSERT_TRUE(inserter.IsValid());

    auto stat = inserter.Insert(*Instance1, true);
    ASSERT_TRUE(stat == SUCCESS);

    stat = inserter.Insert(*Instance2, true);
    ASSERT_TRUE(stat == SUCCESS);

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

    auto& arrayBinderS = stmt.BindArray(2, (int) expectedStringArray.size());
    for (Utf8StringCR arrayElement : expectedStringArray)
        {
        auto& elementBinder = arrayBinderS.AddArrayElement();
        elementBinder.BindText(arrayElement.c_str(), IECSqlBinder::MakeCopy::No);
        }

    IECSqlArrayBinder& arrayBinderI = stmt.BindArray(3, (int) expectedIntArray.size());
    for (int arrayElement : expectedIntArray)
        {
        IECSqlBinder& elementBinder = arrayBinderI.AddArrayElement();
        elementBinder.BindInt(arrayElement);
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

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetNamespacePrefix(schemaIn.GetNamespacePrefix()));

        ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
        ASSERT_EQ(ECN::ECObjectsStatus::Success, contextPtr->AddSchema(*imported));

        ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(contextPtr->GetCache()));
        db.Schemas().CreateECClassViewsInDb();
        };

    ECDbR ecdb = SetupECDb("ImportTwoInARow.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    {
    ECN::ECSchemaPtr schema;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ECN::ECSchema::CreateSchema(schema, "ImportTwoInARow", 0, 0));
    schema->SetNamespacePrefix("tir");

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
        "    <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                     <Strategy>SharedTable</Strategy>"
        "                     <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                     <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Prop1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.01'>"
        "                <MapStrategy>"
        "                     <Options>SharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Prop2' typeName='double' />"
        "        <ECProperty propertyName='Center' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Sub1(Prop1,Center) VALUES(1.1,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3D(1, DPoint3d::From(1.0, 2.0, 3.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    ASSERT_STREQ("INSERT INTO [ts_Base] ([Prop1],[ECInstanceId],ECClassId) VALUES (1.1,?,140);INSERT INTO [ts_Sub1] ([sc02],[sc03],[sc04],[BaseECInstanceId],ECClassId) VALUES (?,?,?,?,140)", stmt.GetNativeSql());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GetX(Center), GetY(Center), GetZ(Center) FROM ts.Sub1 LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1.0, stmt.GetValueDouble(0));
    ASSERT_EQ(2.0, stmt.GetValueDouble(1));
    ASSERT_EQ(3.0, stmt.GetValueDouble(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.Sub1 WHERE GetX(Center) > 0 AND GetY(Center) > 0 AND GetZ(Center) > 0"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

END_ECDBUNITTESTS_NAMESPACE
