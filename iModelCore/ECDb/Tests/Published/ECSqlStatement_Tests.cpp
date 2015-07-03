/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatement_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"
#include <cmath>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSqlSelectTests : public ::testing::Test
    {
    public:
        void SetUpTestDb(ECDbR ecdb)
            {
            BeFileName temporaryDir;
            BeTest::GetHost().GetOutputRoot(temporaryDir);
            BeSQLiteLib::Initialize(temporaryDir);

            BeFileName dbPath;
            BeTest::GetHost().GetDocumentsRoot(dbPath);
            dbPath.AppendToPath(L"DgnDb");
            dbPath.AppendToPath(L"ECSqlStatementTests.ecdb");

            ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsiclass                             Muhammad Hassan                         06/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        void setProductsValues(StandaloneECInstancePtr instance, int ProductId, WCharCP ProductName, double price, bool ProductAvailable)
            {
            instance->SetValue(L"ProductId", ECValue(ProductId));
            instance->SetValue(L"ProductName", ECValue(ProductName));
            instance->SetValue(L"Price", ECValue(price));
            instance->SetValue(L"ProductAvailable", ECValue(ProductAvailable));
            }

        void setEmployeeValues(StandaloneECInstancePtr instance, int EmployeeId, WCharCP FirstName, WCharCP LastName, DateTimeCR DOB, WCharCP Notes)
            {
            instance->SetValue(L"EmployeeId", ECValue(EmployeeId));
            instance->SetValue(L"FirstName", ECValue(FirstName));
            instance->SetValue(L"LastName", ECValue(LastName));
            instance->SetValue(L"DOB", ECValue(DOB));
            instance->SetValue(L"Notes", ECValue(Notes));
            }

        void setSupplierValues(StandaloneECInstancePtr instance, int SupplierId, WCharCP SupplierName, WCharCP ContactName, WCharCP City, WCharCP Country, WCharCP Address, int64_t PostalCode)
            {
            instance->SetValue(L"SupplierId", ECValue(SupplierId));
            instance->SetValue(L"SupplierName", ECValue(SupplierName));
            instance->SetValue(L"ContactName", ECValue(ContactName));
            instance->SetValue(L"City", ECValue(City));
            instance->SetValue(L"Country", ECValue(Country));
            instance->SetValue(L"Address", ECValue(Address));
            instance->SetValue(L"PostalCode", ECValue(PostalCode));
            }

        void setShipperValues(StandaloneECInstancePtr instance, int shipperId, WCharCP ShipperName, int64_t Phone)
            {
            instance->SetValue(L"ShipperId", ECValue(shipperId));
            instance->SetValue(L"ShipperName", ECValue(ShipperName));
            instance->SetValue(L"Phone", ECValue(Phone));
            }

        void setOrderValues(StandaloneECInstancePtr instance, DateTimeCR datetime, int id, bool ProductDelivered)
            {
            instance->SetValue(L"OrderDate", ECValue(datetime));
            instance->SetValue(L"OrderId", ECValue(id));
            instance->SetValue(L"ProductDelivered", ECValue(ProductDelivered));
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                             Muhammad Hassan                         06/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        void setCustomerValues(StandaloneECInstancePtr instance, WCharCP CustomerName, int64_t PostalCode, WCharCP Address, WCharCP ContactName, WCharCP City, WCharCP Country, int CustomerId, StandaloneECInstancePtr OrderInstance1, StandaloneECInstancePtr OrderInstance2, StandaloneECInstancePtr OrderInstance3)
            {
            instance->SetValue(L"CustomerId", ECValue(CustomerId));
            instance->SetValue(L"CustomerName", ECValue(CustomerName));
            instance->SetValue(L"ContactName", ECValue(ContactName));
            instance->SetValue(L"City", ECValue(City));
            instance->SetValue(L"Country", ECValue(Country));
            instance->SetValue(L"PostalCode", ECValue(PostalCode));
            instance->SetValue(L"Address", ECValue(Address));
            instance->AddArrayElements(L"OrderArray", 3);

            ECValue structVal1;
            structVal1.SetStruct(OrderInstance1.get());
            instance->SetValue(L"OrderArray", structVal1, 0);

            ECValue structVal2;
            structVal2.SetStruct(OrderInstance2.get());
            instance->SetValue(L"OrderArray", structVal2, 1);

            ECValue structVal3;
            structVal3.SetStruct(OrderInstance3.get());
            instance->SetValue(L"OrderArray", structVal3, 2);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                             Muhammad Hassan                         06/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        void InsertInstancesForECSqlTestSchema(ECDbR ecdb)
            {
            //create instances of Employee and insert into Db
            ECClassCP EmployeeClass = ecdb.Schemas().GetECClass("ECSqlStatementTests", "Employee");
            ASSERT_TRUE(EmployeeClass != nullptr);
            StandaloneECInstancePtr EmployeeInstance1 = EmployeeClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr EmployeeInstance2 = EmployeeClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr EmployeeInstance3 = EmployeeClass->GetDefaultStandaloneEnabler()->CreateInstance();
            setEmployeeValues(EmployeeInstance1, 1, L"Nancy", L"Davolio", DateTime::GetCurrentTimeUtc(), L"Education includes a BA in psychology...");
            setEmployeeValues(EmployeeInstance2, 2, L"Andrew", L"Fuller", DateTime::GetCurrentTimeUtc(), L"Andrew received his BTS commercial and...");
            setEmployeeValues(EmployeeInstance3, 3, L"Janet", L"Leverling", DateTime::GetCurrentTimeUtc(), L"Janet has a BS degree in chemistry...");
            ECInstanceInserter EmployeeInserter(ecdb, *EmployeeClass);
            ASSERT_TRUE(EmployeeInserter.IsValid());
            ASSERT_EQ(SUCCESS, EmployeeInserter.Insert(*EmployeeInstance1, true));
            ASSERT_EQ(SUCCESS, EmployeeInserter.Insert(*EmployeeInstance2, true));
            ASSERT_EQ(SUCCESS, EmployeeInserter.Insert(*EmployeeInstance3, true));

            //Create and Insert Instances of Products
            ECClassCP ProductsClass = ecdb.Schemas().GetECClass("ECSqlStatementTests", "Products");
            ASSERT_TRUE(ProductsClass != nullptr);
            StandaloneECInstancePtr ProductsInstance1 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance2 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance3 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance4 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance5 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance6 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance7 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance8 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ProductsInstance9 = ProductsClass->GetDefaultStandaloneEnabler()->CreateInstance();
            setProductsValues(ProductsInstance1, 1, L"Pencil", 189.05, true);
            setProductsValues(ProductsInstance2, 2, L"Binder", 999.50, true);
            setProductsValues(ProductsInstance3, 3, L"Pen", 539.73, false);
            setProductsValues(ProductsInstance4, 4, L"Binder", 299.40, true);
            setProductsValues(ProductsInstance5, 5, L"Desk", 150.00, true);
            setProductsValues(ProductsInstance6, 6, L"Pen Set", 255.84, false);
            setProductsValues(ProductsInstance7, 7, L"Pen Set", 479.04, false);
            setProductsValues(ProductsInstance8, 8, L"Pen", 539.73, true);
            setProductsValues(ProductsInstance9, 9, L"Pen", 539.73, true);
            ECInstanceInserter ProductInserter(ecdb, *ProductsClass);
            ASSERT_TRUE(ProductInserter.IsValid());
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance1, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance2, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance3, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance4, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance5, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance6, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance7, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance8, true));
            ASSERT_EQ(SUCCESS, ProductInserter.Insert(*ProductsInstance9, true));

            //Create and Insert Instances of Shipper
            ECClassCP ShipperClass = ecdb.Schemas().GetECClass("ECSqlStatementTests", "Shipper");
            ASSERT_TRUE(ShipperClass != nullptr);
            StandaloneECInstancePtr ShipperInstance1 = ShipperClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ShipperInstance2 = ShipperClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr ShipperInstance3 = ShipperClass->GetDefaultStandaloneEnabler()->CreateInstance();
            setShipperValues(ShipperInstance1, 1, L"Rio Grand", 19783482);
            setShipperValues(ShipperInstance2, 2, L"Rue Perisnon", 26422096);
            setShipperValues(ShipperInstance3, 3, L"Salguero", 39045213);
            ECInstanceInserter ShipperInserter(ecdb, *ShipperClass);
            ASSERT_TRUE(ShipperInserter.IsValid());
            ASSERT_EQ(SUCCESS, ShipperInserter.Insert(*ShipperInstance1, true));
            ASSERT_EQ(SUCCESS, ShipperInserter.Insert(*ShipperInstance2, true));
            ASSERT_EQ(SUCCESS, ShipperInserter.Insert(*ShipperInstance3, true));

            //Create and Insert Instances of Supplier
            ECClassCP SupplierClass = ecdb.Schemas().GetECClass("ECSqlStatementTests", "Supplier");
            ASSERT_TRUE(SupplierClass != nullptr);
            StandaloneECInstancePtr SupplierInstance1 = SupplierClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr SupplierInstance2 = SupplierClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr SupplierInstance3 = SupplierClass->GetDefaultStandaloneEnabler()->CreateInstance();
            setSupplierValues(SupplierInstance1, 1, L"John", L"Snow", L"CA", L"USA", L"5089 CALERO AVENUE", 95123);
            setSupplierValues(SupplierInstance2, 2, L"Trion", L"Lannistor", L"NC", L"USA", L"198 FAYETTEVILLE ROAD", 27514);
            setSupplierValues(SupplierInstance3, 3, L"Stannis", L"Brathion", L"MD", L"USA", L"1598 PICCARD DRIVE", 20850);
            ECInstanceInserter SupplierInserter(ecdb, *SupplierClass);
            ASSERT_TRUE(SupplierInserter.IsValid());
            ASSERT_EQ(SUCCESS, SupplierInserter.Insert(*SupplierInstance1, true));
            ASSERT_EQ(SUCCESS, SupplierInserter.Insert(*SupplierInstance2, true));
            ASSERT_EQ(SUCCESS, SupplierInserter.Insert(*SupplierInstance3, true));

            //Create instances of Order Class and they will be only specific to a Customer no Standalone order.
            ECClassCP OrderClass = ecdb.Schemas().GetECClass("ECSqlStatementTests", "Order");
            ASSERT_TRUE(OrderClass != nullptr);

            StandaloneECInstancePtr OrderInstance1 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance2 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance3 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance4 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance5 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance6 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance7 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance8 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr OrderInstance9 = OrderClass->GetDefaultStandaloneEnabler()->CreateInstance();

            setOrderValues(OrderInstance1, DateTime::GetCurrentTimeUtc(), 1, true);
            setOrderValues(OrderInstance2, DateTime::GetCurrentTimeUtc(), 2, false);
            setOrderValues(OrderInstance3, DateTime::GetCurrentTimeUtc(), 3, false);
            setOrderValues(OrderInstance4, DateTime::GetCurrentTimeUtc(), 4, true);
            setOrderValues(OrderInstance5, DateTime::GetCurrentTimeUtc(), 5, true);
            setOrderValues(OrderInstance6, DateTime::GetCurrentTimeUtc(), 6, true);
            setOrderValues(OrderInstance7, DateTime::GetCurrentTimeUtc(), 7, false);
            setOrderValues(OrderInstance8, DateTime::GetCurrentTimeUtc(), 8, false);
            setOrderValues(OrderInstance9, DateTime::GetCurrentTimeUtc(), 9, true);
            ECInstanceInserter inserter(ecdb, *OrderClass);
            ASSERT_TRUE(inserter.IsValid());
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance1, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance2, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance3, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance4, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance5, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance6, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance7, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance8, true));
            ASSERT_EQ(SUCCESS, inserter.Insert(*OrderInstance9, true));

            ECClassCP CustomerClass = ecdb.Schemas().GetECClass("ECSqlStatementTests", "Customer");
            ASSERT_TRUE(CustomerClass != nullptr);

            StandaloneECInstancePtr CustomerInstance1 = CustomerClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr CustomerInstance2 = CustomerClass->GetDefaultStandaloneEnabler()->CreateInstance();
            StandaloneECInstancePtr CustomerInstance3 = CustomerClass->GetDefaultStandaloneEnabler()->CreateInstance();
            setCustomerValues(CustomerInstance1, L"Charles Baron", 78701, L"44 PRINCESS GATE, HYDE PARK", L"Brathion", L"SAN JOSE", L"USA", 1, OrderInstance1, OrderInstance2, OrderInstance3);
            setCustomerValues(CustomerInstance2, L"Gunther Spielmann", 22090, L"5063 RICHMOND MALL", L"SPIELMANN", L"AUSTIN", L"USA", 2, OrderInstance4, OrderInstance5, OrderInstance6);
            setCustomerValues(CustomerInstance3, L"A.D.M. Bryceson", 93274, L"3-2-7 ETCHUJMA, KOTO-KU", L"Adm", L"SAN JOSE", L"USA", 3, OrderInstance7, OrderInstance8, OrderInstance9);

            ECInstanceInserter CustomerInserter(ecdb, *CustomerClass);
            ASSERT_TRUE(CustomerInserter.IsValid());
            ASSERT_EQ(SUCCESS, CustomerInserter.Insert(*CustomerInstance1));
            ASSERT_EQ(SUCCESS, CustomerInserter.Insert(*CustomerInstance2));
            ASSERT_EQ(SUCCESS, CustomerInserter.Insert(*CustomerInstance3));

            ECRelationshipClassCP EmployeeCustomer = ecdb.Schemas().GetECClass("ECSqlStatementTests", "EmployeeCustomer")->GetRelationshipClassCP();
            ASSERT_TRUE(EmployeeCustomer != nullptr);

            StandaloneECRelationshipInstancePtr EmployeeCustomerInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*EmployeeCustomer)->CreateRelationshipInstance();
            ECInstanceInserter EmployeeCustomerInserter(ecdb, *EmployeeCustomer);
            ASSERT_TRUE(EmployeeCustomerInserter.IsValid());
            EmployeeCustomerInstance->SetSource(EmployeeInstance1.get());
            EmployeeCustomerInstance->SetTarget(CustomerInstance1.get());
            EmployeeCustomerInstance->SetInstanceId(L"source->target");
            ASSERT_EQ(SUCCESS, EmployeeCustomerInserter.Insert(*EmployeeCustomerInstance));

            //Create and Insert Instances of Relationship ProductSupplier
            ECRelationshipClassCP ProductSupplier = ecdb.Schemas().GetECClass("ECSqlStatementTests", "ProductSupplier")->GetRelationshipClassCP();
            ASSERT_TRUE(ProductSupplier != nullptr);
            StandaloneECRelationshipInstancePtr ProductSupplierInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*ProductSupplier)->CreateRelationshipInstance();
            ECInstanceInserter ProductSupplierInserter(ecdb, *ProductSupplier);
            ASSERT_TRUE(ProductSupplierInserter.IsValid());
            ProductSupplierInstance->SetSource(ProductsInstance1.get());
            ProductSupplierInstance->SetTarget(SupplierInstance1.get());
            ProductSupplierInstance->SetInstanceId(L"source->target");
            ASSERT_EQ(SUCCESS, ProductSupplierInserter.Insert(*ProductSupplierInstance));
            ProductSupplierInstance->SetSource(ProductsInstance2.get());
            ProductSupplierInstance->SetTarget(SupplierInstance2.get());
            ProductSupplierInstance->SetInstanceId(L"source->target");
            ASSERT_EQ(SUCCESS, ProductSupplierInserter.Insert(*ProductSupplierInstance));
            ProductSupplierInstance->SetSource(ProductsInstance3.get());
            ProductSupplierInstance->SetTarget(SupplierInstance3.get());
            ProductSupplierInstance->SetInstanceId(L"source->target");
            ASSERT_EQ(SUCCESS, ProductSupplierInserter.Insert(*ProductSupplierInstance));
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

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, PopulateECSql_TestDbWithTestData)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create ("ECSqlStatementTests.ecdb", L"ECSqlStatementTests.01.00.ecschema.xml", false);
    InsertInstancesForECSqlTestSchema (ecdbr);
    ecdbr.CloseDb ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, UnionTests)
    {
    ECDb ecdb;
    SetUpTestDb (ecdb);
    int rowCount;
    Utf8String ExpectedColumnValues;
    Utf8String ActualColumnValues;
    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM (SELECT ContactName, Address, City, PostalCode, Country FROM ECST.Supplier UNION ALL SELECT ContactName, Address, City, PostalCode, Country FROM ECST.Customer)"));
    ASSERT_EQ (stmt.Step (), ECSqlStepStatus::HasRow);
    int count = stmt.GetValueInt (0);
    EXPECT_EQ (6, count);
    stmt.Finalize ();

    //Select Statement containing Union All Clause and also Order By clause
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ContactName, Address, City, PostalCode, Country FROM ECST.Supplier UNION ALL SELECT ContactName, Address, City, PostalCode, Country FROM ECST.Customer ORDER BY PostalCode"));
    rowCount = 0;
    ExpectedColumnValues = "Brathion-SPIELMANN-Lannistor-Brathion-Adm-Snow-";
    ActualColumnValues;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        ActualColumnValues.append (stmt.GetValueText (0));
        ActualColumnValues.append ("-");
        rowCount++;
        }
    ASSERT_EQ (ActualColumnValues, ExpectedColumnValues) << "Expected ContactNames OrderBy ECInstanceId Doesn't match Original ContactNames";
    ASSERT_EQ (6, rowCount);
    stmt.Finalize ();

    //Select Statement using UNION Clause, so we should get only distinct results
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT City FROM ECST.Supplier UNION SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    Utf8String ExpectedCityValues = "AUSTIN-CA-MD-NC-SAN JOSE-";
    Utf8String ActualCityValues;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        ActualCityValues.append (stmt.GetValueText (0));
        ActualCityValues.append ("-");
        rowCount++;
        }
    ASSERT_EQ (ActualCityValues, ExpectedCityValues) << "Expected city Names Doesn't match Actual values";
    ASSERT_EQ (5, rowCount);
    stmt.Finalize ();

    //Select Statement Using UNION ALL Clause so we should get even Duplicate Results
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT City FROM ECST.Supplier UNION ALL SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    ExpectedCityValues = "AUSTIN-CA-MD-NC-SAN JOSE-SAN JOSE-";
    ActualCityValues = "";
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        ActualCityValues.append (stmt.GetValueText (0));
        ActualCityValues.append ("-");
        rowCount++;
        }
    ASSERT_EQ (ActualCityValues, ExpectedCityValues) << "Expected city Names Doesn't match Actual Names";
    ASSERT_EQ (6, rowCount);
    stmt.Finalize ();

    //use Custom Scaler function in union query
    PowSqlFunction func;
    ASSERT_EQ (0, ecdb.AddFunction (func));
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "Select POW(ECInstanceId, 2), GetECClassId() ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW(ECInstanceId, 2), GetECClassId() ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
    rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        int base = stmt.GetValueInt (2);
        ASSERT_EQ (std::pow (base, 2), stmt.GetValueInt (0));
        rowCount++;
        }
    ASSERT_EQ (6, rowCount);
    stmt.Finalize ();

    //use aggregate function in Union Query
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT Count(*), AVG(PostalCode) FROM (SELECT PostalCode FROM ECST.Supplier UNION ALL SELECT PostalCode FROM ECST.Customer)"));
    ASSERT_EQ (stmt.Step (), ECSqlStepStatus::HasRow);
    ASSERT_EQ (6, stmt.GetValueInt (0));
    ASSERT_EQ (56258, stmt.GetValueInt (1));
    stmt.Finalize ();

    //Use GROUP BY clause in Union Query
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ECClassId, COUNT(*) FROM (SELECT GetECClassId() ECClassId, ECInstanceId FROM ECST.Supplier UNION ALL SELECT GetECClassId() ECClassId, ECInstanceId FROM ECST.Customer) GROUP BY ECClassId ORDER BY ECClassId"));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::HasRow);
    //ASSERT_EQ (128, stmt.GetValueInt (0));
    ASSERT_EQ (3, stmt.GetValueInt (1));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::HasRow);
    //ASSERT_EQ (134, stmt.GetValueInt (0));
    ASSERT_EQ (3, stmt.GetValueInt (1));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::Done);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, ExceptTests)
    {
    ECDb ecdb;
    SetUpTestDb (ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ContactName FROM ECST.Supplier EXCEPT SELECT ContactName FROM ECST.Customer"));
    int rowCount = 0;
    Utf8String expectedContactNames = "Lannistor-Snow-";
    Utf8String actualContactNames;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        actualContactNames.append (stmt.GetValueText (0));
        actualContactNames.append ("-");
        rowCount++;
        }
    ASSERT_EQ (expectedContactNames, actualContactNames);
    ASSERT_EQ (2, rowCount);
    stmt.Finalize ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, IntersectTests)
    {
    ECDb ecdb;
    SetUpTestDb (ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ContactName FROM ECST.Supplier INTERSECT SELECT ContactName FROM ECST.Customer ORDER BY ContactName"));
    int rowCount = 0;
    Utf8String expectedContactNames = "Brathion";
    Utf8String actualContactNames;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        actualContactNames.append (stmt.GetValueText (0));
        rowCount++;
        }
    ASSERT_EQ (expectedContactNames, actualContactNames);
    ASSERT_EQ (1, rowCount);
    stmt.Finalize ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, NestedSelectStatementsTests)
    {
    Utf8CP ecSqlSelect = "SELECT ProductName From ECST.Products WHERE Price = ?";
    ECDb ecdb;
    SetUpTestDb (ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ProductName, Price FROM ECST.Products WHERE Price > (SELECT AVG(Price) From ECST.Products) AND Price < 500"));
    ASSERT_EQ (stmt.Step (), ECSqlStepStatus::HasRow);

    ECSqlStatementCache cache (1);
    CachedECSqlStatementPtr cashedStmt = cache.GetPreparedStatement (ecdb, ecSqlSelect);
    ASSERT_TRUE (cashedStmt != nullptr);
    ASSERT_TRUE (cashedStmt->IsPrepared ());
    ASSERT_EQ (ECSqlStatus::Success, cashedStmt->BindDouble (1, stmt.GetValueDouble (1))) << "Binding Double value failed";
    ASSERT_TRUE (cashedStmt->Step () == ECSqlStepStatus::HasRow);
    ASSERT_EQ ((Utf8String)stmt.GetValueText (0), (Utf8String)cashedStmt->GetValueText (0));
    stmt.Finalize ();

    //Using GetECClassId in Nested Select statement
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ECClassId, COUNT(*) FROM (SELECT GetECClassId() ECClassId, ECInstanceId FROM ECST.Supplier UNION ALL SELECT GetECClassId() ECClassId, ECInstanceId FROM ECST.Customer) GROUP BY ECClassId ORDER BY ECClassId"));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::HasRow);
    ASSERT_EQ (129, stmt.GetValueInt (0));
    ASSERT_EQ (3, stmt.GetValueInt (1));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::HasRow);
    ASSERT_EQ (135, stmt.GetValueInt (0));
    ASSERT_EQ (3, stmt.GetValueInt (1));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::Done);
    stmt.Finalize ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, TestPredicateFunctionsInNestedSelectStatement)
    {
    ECDb ecdb;
    SetUpTestDb (ecdb);
    ECSqlStatement stmt;

    //Using Predicate function in nexted select statement
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT AVG(Price) FROM ECST.Products WHERE Price IN (SELECT Price FROM ECST.Products WHERE Price < (SELECT AVG(Price) FROM ECST.Products WHERE ProductAvailable))"));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::HasRow);
    ASSERT_EQ (223, (int)stmt.GetValueDouble (0));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::Done);
    stmt.Finalize ();

    //Using NOT operator with predicate function in Nested Select statement
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT AVG(Price) FROM ECST.Products WHERE Price IN (SELECT Price FROM ECST.Products WHERE Price > (SELECT AVG(Price) FROM ECST.Products WHERE NOT ProductAvailable))"));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::HasRow);
    ASSERT_EQ (619, (int)stmt.GetValueDouble (0));
    ASSERT_TRUE (stmt.Step () == ECSqlStepStatus::Done);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSqlSelectTests, GroupByClauseTests)
    {
    ECDb ecdb;
    SetUpTestDb (ecdb);
    Utf8String expectedProductsNames;
    Utf8String actualProductsNames;
    double ExpectedSumOfAvgPrices;
    double actualSumOfAvgPrices;
    ECSqlStatement stmt;
    //use of simple GROUP BY clause to find AVG(Price) from the Product table
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ProductName, AVG(Price) FROM ECST.Products GROUP BY ProductName ORDER BY ProductName"));
    expectedProductsNames = "Binder-Desk-Pen-Pen Set-Pencil-";
    actualProductsNames;
    ExpectedSumOfAvgPrices = 1895.67;
    actualSumOfAvgPrices = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        actualProductsNames.append (stmt.GetValueText (0));
        actualProductsNames.append ("-");
        actualSumOfAvgPrices += stmt.GetValueDouble (1);
        }
    ASSERT_EQ (expectedProductsNames, actualProductsNames);
    ASSERT_EQ (ExpectedSumOfAvgPrices, actualSumOfAvgPrices);
    stmt.Finalize ();

    //using HAVING clause with GROUP BY clause
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ProductName, AVG(Price) FROM ECST.Products GROUP BY ProductName Having AVG(Price)>300.00 ORDER BY ProductName"));
    expectedProductsNames = "Binder-Pen-Pen Set-";
    actualProductsNames = "";
    ExpectedSumOfAvgPrices = 1556.62;
    actualSumOfAvgPrices = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        actualProductsNames.append (stmt.GetValueText (0));
        actualProductsNames.append ("-");
        actualSumOfAvgPrices += stmt.GetValueDouble (1);
        }
    ASSERT_EQ (expectedProductsNames, actualProductsNames);
    ASSERT_EQ ((int)ExpectedSumOfAvgPrices, (int)actualSumOfAvgPrices);
    stmt.Finalize ();

    //combined Use of GROUP BY, HAVING and WHERE Clause
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ProductName, AVG(Price) FROM ECST.Products WHERE Price<500 GROUP BY ProductName Having AVG(Price)>200.00 ORDER BY ProductName"));
    expectedProductsNames = "Binder-Pen Set-";
    actualProductsNames = "";
    ExpectedSumOfAvgPrices = 666.84;
    actualSumOfAvgPrices = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        actualProductsNames.append (stmt.GetValueText (0));
        actualProductsNames.append ("-");
        actualSumOfAvgPrices += stmt.GetValueDouble (1);
        }
    ASSERT_EQ (expectedProductsNames, actualProductsNames);
    ASSERT_EQ ((int)ExpectedSumOfAvgPrices, (int)actualSumOfAvgPrices);
    stmt.Finalize ();

    //GROUP BY Clause with more then one parameters
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ProductName, AVG(Price), COUNT(ProductName) FROM ECST.Products GROUP BY ProductName, Price HAVING COUNT(ProductName)>1"));
    ASSERT_EQ (stmt.Step (), ECSqlStepStatus::HasRow);
    ASSERT_EQ ("Pen", (Utf8String)stmt.GetValueText (0));
    ASSERT_EQ (539, (int)stmt.GetValueDouble (1));
    ASSERT_EQ (3, stmt.GetValueInt (2));
    ASSERT_FALSE (stmt.Step () != ECSqlStepStatus::Done);
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> CreateECInstance_S4 (ECDbR ecdb, int n)
    {
    ECClassCP s4 = ecdb. Schemas ().GetECClass ("NestedStructArrayTest", "S4");
    BeAssert (s4 != nullptr);

    bvector<IECInstancePtr> vect;
    for (auto j = 0; j < n; j++)
        {
        auto inst = s4->GetDefaultStandaloneEnabler ()->CreateInstance ();
        inst->SetValue (L"I", ECValue (j));
        inst->SetValue (L"T", ECValue (L"testData_S4"));
        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> CreateECInstance_S3 (ECDbR ecdb, int n)
    {
    int m = n + 1;
    ECClassCP s3 = ecdb. Schemas ().GetECClass ("NestedStructArrayTest", "S3");
    BeAssert (s3 != nullptr);

    bvector<IECInstancePtr> vect;
    for (auto j = 0; j < n; j++)
        {
        auto inst = s3->GetDefaultStandaloneEnabler ()->CreateInstance ();
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"I", ECValue (j)));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"T", ECValue (L"testData_S3")));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->AddArrayElements (L"S4ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S4 (ecdb, m))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"S4ARRAY", elmV, v++));
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> CreateECInstance_S2 (ECDbR ecdb, int n)
    {
    int m = n + 1;
    ECClassCP s2 = ecdb. Schemas ().GetECClass ("NestedStructArrayTest", "S2");
    BeAssert (s2 != nullptr);

    bvector<IECInstancePtr> vect;
    for (auto j = 0; j < n; j++)
        {
        auto inst = s2->GetDefaultStandaloneEnabler ()->CreateInstance ();
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"I", ECValue (j)));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"T", ECValue (L"testData_S2")));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->AddArrayElements (L"S3ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S3 (ecdb, m))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"S3ARRAY", elmV, v++));
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> CreateECInstance_S1 (ECDbR ecdb, int n)
    {
    int m = n + 1;
    ECClassCP s1 = ecdb. Schemas ().GetECClass ("NestedStructArrayTest", "S1");
    BeAssert (s1 != nullptr);

    bvector<IECInstancePtr> vect;
    for (auto j = 0; j < n; j++)
        {
        auto inst = s1->GetDefaultStandaloneEnabler ()->CreateInstance ();
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"I", ECValue (j)));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"T", ECValue (L"testData_S1")));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->AddArrayElements (L"S2ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S2 (ecdb, m))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"S2ARRAY", elmV, v++));
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> CreateECInstance_ClassP (ECDbR ecdb, int n)
    {
    int m = n + 1;
    ECClassCP classP = ecdb. Schemas ().GetECClass ("NestedStructArrayTest", "ClassP");
    BeAssert (classP != nullptr);

    bvector<IECInstancePtr> vect;
    for (auto j = 0; j < n; j++)
        {
        auto inst = classP->GetDefaultStandaloneEnabler ()->CreateInstance ();
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"I", ECValue (j)));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"T", ECValue (L"testData_ClassP")));
        BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->AddArrayElements (L"S1ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S1 (ecdb, m))
            {
            ECValue elmV; 
            elmV.SetStruct (elm.get ());
            BeAssert (ECObjectsStatus::ECOBJECTS_STATUS_Success == inst->SetValue (L"S1ARRAY", elmV, v++));
            }

        vect.push_back (inst);
        }

    return vect;
    }
//
//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_InsertStructArray)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp ("NestedStructArrayTest.ecdb", L"NestedStructArrayTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
    
    auto in = CreateECInstance_ClassP (ecdb, 1);

    WString inXml, outXml;
    for (auto inst : in)
        {
        ECInstanceInserter inserter (ecdb, inst->GetClass ());
        auto st = inserter.Insert (*inst);
        ASSERT_TRUE (st == BentleyStatus::SUCCESS);
        inst->WriteToXmlString (inXml, true, true);
        inXml += L"\r\n";
        }
    
    bvector<IECInstancePtr> out;

    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare (ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader (stmt);
    while (stmt.Step () == ECSqlStepStatus::HasRow)
        {
        auto inst = classPReader.GetInstance ();
        out.push_back (inst);
        inst->WriteToXmlString (outXml, true, true);
        outXml += L"\r\n";
        }
    ASSERT_EQ (in.size (), out.size ());
    ASSERT_TRUE (inXml == outXml);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_DeleteStructArray)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp ("NestedStructArrayTest.ecdb", L"NestedStructArrayTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    auto in = CreateECInstance_ClassP (ecdb, 1);

    for (auto inst : in)
        {
        ECInstanceInserter inserter (ecdb, inst->GetClass ());
        auto st = inserter.Insert (*inst);
        ASSERT_TRUE (st == BentleyStatus::SUCCESS);
        }


    ECClassCP classP = ecdb. Schemas ().GetECClass ("NestedStructArrayTest", "ClassP");
    BeAssert (classP != nullptr);

    ECInstanceDeleter deleter(ecdb, *classP);
    for (auto& inst : in)
        {
        ASSERT_TRUE (deleter.Delete (*inst) == BentleyStatus::SUCCESS);
        }

    bvector<IECInstancePtr> out;
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare (ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader (stmt);
    ASSERT_TRUE (stmt.Step () != ECSqlStepStatus::HasRow);
    }

//---------------------------------------------------------------------------------------
// Sandbox for debugging ECSqlStatement
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
#ifdef IGNORE_IT
TEST_F (ECSqlTestFixture, Debug)
    {
    // Create and populate a sample project
    auto& ecdb = SetUp ("test.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), 0);

    ECSqlStatement stmt;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "INSERT INTO ecsql.PSA (ECInstanceId) VALUES (NULL)"));
    ECInstanceKey psaId;
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step (psaId));

    stmt.Finalize ();
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "INSERT INTO ecsql.THBase (ECInstanceId) VALUES (NULL)"));
    ECInstanceKey thbaseId;
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step (thbaseId));

    stmt.Finalize ();

    Utf8String ecsqlStr;
    ecsqlStr.Sprintf ("INSERT INTO ecsql.PSAHasTHBase_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (%lld, ?, %lld, ?);",
        psaId.GetECInstanceId ().GetValue (),
        thbaseId.GetECInstanceId ().GetValue ());

    auto stat = stmt.Prepare (ecdb, ecsqlStr.c_str ());
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));

    stat = stmt.BindInt64 (1, 135LL);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));
    stat = stmt.BindInt64 (2, 142LL);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));
    
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
    }
#endif
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_Prepare)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "SELECT I, PStruct_Array FROM ecsql.PSA");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.GetValueInt (0);
        //WIP: Error handling for unstepped statements not done yet. Once done uncomment below line
        //and remove the other one
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());
        //ASSERT_EQ ((int) ECSqlStatus::UserError, (int) statement.GetLastStatus ());

        statement.GetValueArray (1);
        //WIP: Error handling for unstepped statements not done yet. Once done uncomment below line
        //and remove the other one
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());
        //ASSERT_EQ ((int) ECSqlStatus::UserError, (int) statement.GetLastStatus ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_ECInstanceIdColumnInfo)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

    auto ecsql = "SELECT c1.ECInstanceId, c2.ECInstanceId FROM ecsql.PSA c1, ecsql.P c2 LIMIT 1";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) statement.Step ());
    auto const& value1 = statement.GetValue (0);
    auto const& columnInfo1 = value1.GetColumnInfo ();

    ASSERT_FALSE (value1.IsNull ());
    ASSERT_FALSE (columnInfo1.IsGeneratedProperty ());
    ASSERT_STREQ (L"ECSqlSystemProperties", columnInfo1.GetProperty ()->GetClass ().GetName ().c_str ());
    ASSERT_STREQ ("c1", columnInfo1.GetRootClassAlias ());
    ASSERT_STREQ (L"PSA", columnInfo1.GetRootClass ().GetName ().c_str ());

    auto const& value2 = statement.GetValue (1);
    auto const& columnInfo2 = value2.GetColumnInfo ();

    ASSERT_FALSE (value2.IsNull ());
    ASSERT_FALSE (columnInfo2.IsGeneratedProperty ());
    ASSERT_STREQ (L"ECSqlSystemProperties", columnInfo2.GetProperty ()->GetClass ().GetName ().c_str ());
    ASSERT_STREQ ("c2", columnInfo2.GetRootClassAlias ());
    ASSERT_STREQ (L"P", columnInfo2.GetRootClass ().GetName ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_StructArrayInsert)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(?, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    
    statement.BindInt64 (1, 1000);
    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray (2, (uint32_t) count);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "BindArray failed: " << statement.GetLastStatusMessage ();
    for (int i = 0; i < count; i++)
        {        
        auto& structBinder = arrayBinder.AddArrayElement ().BindStruct ();
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "AddArrayElement failed: " << statement.GetLastStatusMessage ();
        auto stat = structBinder.GetMember (L"d").BindDouble (i * PI);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"i").BindInt (i * 2);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"l").BindInt64 (i * 3);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"p2d").BindPoint2D (DPoint2d::From (i, i + 1));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"p3d").BindPoint3D (DPoint3d::From (i, i + 1, i + 2));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        }

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_StructArrayUpdate)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "UPDATE  ONLY ecsql.PSA SET L = ?,  PStruct_Array = ? WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    statement.BindInt (3, 123);
    statement.BindInt64 (1, 1000);
    //add three array elements
    const uint32_t arraySize = 3;
    auto& arrayBinder = statement.BindArray (2, arraySize);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)statement.GetLastStatus ()) << "BindArray failed: " << statement.GetLastStatusMessage ();
    for (int i = 0; i < arraySize; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement ().BindStruct ();
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)statement.GetLastStatus ()) << "AddArrayElement failed: " << statement.GetLastStatusMessage ();
        auto stat = structBinder.GetMember (L"d").BindDouble (i * PI);
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"i").BindInt (i * 2);
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"l").BindInt64 (i * 3);
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"p2d").BindPoint2D (DPoint2d::From (i, i + 1));
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember (L"p3d").BindPoint3D (DPoint3d::From (i, i + 1, i + 2));
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        }

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_StructArrayDelete)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    statement.BindInt (1, 123);

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_BindECInstanceId)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECInstanceKey pKey;
    ECInstanceKey psaKey;

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.P (ECInstanceId) VALUES(NULL)");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (pKey));
        
        statement.Finalize ();
        stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSA (ECInstanceId) VALUES(NULL)");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (psaKey));
        ecdb.SaveChanges ();
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindId (1, psaKey.GetECInstanceId ()));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindId (2, pKey.GetECInstanceId ()));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));

        ASSERT_EQ (pKey.GetECInstanceId ().GetValue (), key.GetECInstanceId ().GetValue ());
        ecdb.AbandonChanges ();
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt (1, (int) psaKey.GetECInstanceId ().GetValue ()));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt (2, (int) pKey.GetECInstanceId ().GetValue ()));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));

        ASSERT_EQ (pKey.GetECInstanceId ().GetValue (), key.GetECInstanceId ().GetValue ());
        ecdb.AbandonChanges ();
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt64 (1, psaKey.GetECInstanceId ().GetValue ()));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt64 (2, pKey.GetECInstanceId ().GetValue ()));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));

        ASSERT_EQ (pKey.GetECInstanceId ().GetValue (), key.GetECInstanceId ().GetValue ());
        ecdb.AbandonChanges ();
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        WChar idStrW[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
        ECInstanceIdHelper::ToString (idStrW, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, psaKey.GetECInstanceId ());
        Utf8String psaIdStr (idStrW);
        ECInstanceIdHelper::ToString (idStrW, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, pKey.GetECInstanceId ());
        Utf8String pIdStr (idStrW);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindText (1, psaIdStr.c_str (), IECSqlBinder::MakeCopy::No));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindText (2, pIdStr.c_str (), IECSqlBinder::MakeCopy::No));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));

        ASSERT_EQ (pKey.GetECInstanceId ().GetValue (), key.GetECInstanceId ().GetValue ());
        ecdb.AbandonChanges ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_BindSourceAndTargetECInstanceId)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        {
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindId (1, ECInstanceId (111LL)));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindId (2, ECInstanceId (222LL)));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));
        }

        {
        statement.Reset ();
        statement.ClearBindings ();

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt (1, 1111));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt (2, 2222));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));
        }

        {
        statement.Reset ();
        statement.ClearBindings ();

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt64 (1, 11111LL));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindInt64 (2, 22222LL));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));
        }

        {
        statement.Reset ();
        statement.ClearBindings ();

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindText (1, "111111", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindText (2, "222222", IECSqlBinder::MakeCopy::No));

        ECInstanceKey key;
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (key));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_BindPrimitiveArray)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
        
    std::vector<int> expectedIntArray = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::vector<Utf8String> expectedStringArray = { "1", "2", "3", "4", "5", "6", "7", "8" };

    ECInstanceKey ecInstanceKey;
        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PA (I_Array,S_Array) VALUES(:ia,:sa)");
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

        auto& arrayBinderI = statement.BindArray (1, (int) expectedIntArray.size ());
        for (int arrayElement : expectedIntArray)
            {
            auto& elementBinder = arrayBinderI.AddArrayElement ();
            elementBinder.BindInt (arrayElement);
            }

        auto& arrayBinderS = statement.BindArray (2, (int) expectedStringArray.size ());
        for (Utf8StringCR arrayElement : expectedStringArray)
            {
            auto& elementBinder = arrayBinderS.AddArrayElement ();
            elementBinder.BindText (arrayElement.c_str (), IECSqlBinder::MakeCopy::No);
            }


        auto stepStat = statement.Step (ecInstanceKey);
        ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStat);
        }

            {
            ECSqlStatement statement;
            auto stat = statement.Prepare (ecdb, "SELECT I_Array, S_Array FROM ONLY ecsql.PA WHERE ECInstanceId = ?");
            ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);
            statement.BindId (1, ecInstanceKey.GetECInstanceId ());

            auto stepStat = statement.Step ();
            ASSERT_EQ ((int)ECSqlStepStatus::HasRow, (int)stepStat);

            IECSqlArrayValue const& intArray = statement.GetValueArray (0);
            size_t expectedIndex = 0;
            for (IECSqlValue const* arrayElement : intArray)
                {
                int actualArrayElement = arrayElement->GetInt ();
                ASSERT_EQ ((int)ECSqlStatus::Success, (int)statement.GetLastStatus ());
                ASSERT_EQ (expectedIntArray[expectedIndex], actualArrayElement);
                expectedIndex++;
                }

            IECSqlArrayValue const& stringArray = statement.GetValueArray (1);
            expectedIndex = 0;
            for (IECSqlValue const* arrayElement : stringArray)
                {
                auto actualArrayElement = arrayElement->GetText ();
                ASSERT_EQ ((int)ECSqlStatus::Success, (int)statement.GetLastStatus ());
                ASSERT_STREQ (expectedStringArray[expectedIndex].c_str (), actualArrayElement);
                expectedIndex++;
                }
            }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_BindDateTimeArray_Insert)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PA (Dt_Array, DtUtc_Array) VALUES(:dt,:dtutc)");
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

    std::vector<DateTime> testDates = { DateTime (DateTime::Kind::Utc, 2014, 07, 07, 12, 0),
        DateTime (DateTime::Kind::Unspecified, 2014, 07, 07, 12, 0),
        DateTime (DateTime::Kind::Local, 2014, 07, 07, 12, 0) };

    auto& arrayBinderDt = statement.BindArray (1, 3);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

    for (DateTimeCR testDate : testDates)
        {
        auto& elementBinder = arrayBinderDt.AddArrayElement ();
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

        auto expectedStat = testDate.GetInfo ().GetKind () == DateTime::Kind::Local ? ECSqlStatus::UserError : ECSqlStatus::Success;
        ASSERT_EQ ((int) expectedStat, (int) elementBinder.BindDateTime (testDate));
        }

    auto& arrayBinderDtUtc = statement.BindArray (2, 3);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

    for (DateTimeCR testDate : testDates)
        {
        auto& elementBinder = arrayBinderDtUtc.AddArrayElement ();
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

        auto expectedStat = testDate.GetInfo ().GetKind () == DateTime::Kind::Utc ? ECSqlStatus::Success : ECSqlStatus::UserError;
        ASSERT_EQ ((int) expectedStat, (int) elementBinder.BindDateTime (testDate));
        }

    auto stepStat = statement.Step ();
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStat);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_BindPrimArrayWithOutOfBoundsLength)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.ABounded (Prim_Array_Bounded) VALUES(?)");
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    auto bindArrayValues = [&statement] (int count)
        {
        statement.Reset ();
        statement.ClearBindings ();

        auto& arrayBinder = statement.BindArray (1, 5);
        for (int i = 0; i < count; i++)
            {
            auto& elementBinder = arrayBinder.AddArrayElement ();
            elementBinder.BindInt (i);
            }

        return statement.Step ();
        };

    //first: array size to bind. second: Expected to succeed
    const std::vector<std::pair<int, bool>> testArrayCounts = { { 0, false }, { 2, false }, { 5, true }, { 7, true }, { 10, true }, 
            { 20, true } }; //Bug in ECObjects: ignores maxoccurs and always interprets it as unbounded.

    //TODO: Currently minoccurs/maxoccurs are disabled in ECDb because of legacy data support.
    //Once this changes, we need to uncomment the respective code again.
    for (auto const& testArrayCountItem : testArrayCounts)
        {
        int testArrayCount = testArrayCountItem.first;
        //bool expectedToSucceed = testArrayCountItem.second;
        auto stepStat = bindArrayValues (testArrayCount);
        //if (expectedToSucceed)
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStat) << "Binding array of length " << testArrayCount << " is expected to succceed for array parameter with minOccurs=5 and maxOccurs=10. Error message: " << statement.GetLastStatusMessage ();
        //else
        //    ASSERT_EQ ((int) ECSqlStepStatus::Error, (int) stepStat) << "Binding array of length " << testArrayCount << " is expected to fail for array parameter with minOccurs=5 and maxOccurs=10";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_InsertWithStructBinding)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    auto testFunction = [this, &ecdb] (Utf8CP insertECSql, bool bindExpectedToSucceed, int structParameterIndex, Utf8CP structValueJson, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue (Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse (structValueJson, expectedStructValue);
        ASSERT_TRUE (parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, insertECSql);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of ECSQL '" << insertECSql << "' failed: " << statement.GetLastStatusMessage ();

        auto& binder = statement.GetBinder (structParameterIndex);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson (bindStatus, statement, expectedStructValue, binder);
        ASSERT_EQ (bindExpectedToSucceed, bindStatus == SUCCESS);
        if (!bindExpectedToSucceed)
            return; 

        ECInstanceKey ecInstanceKey;
        auto stepStat = statement.Step (ecInstanceKey);
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStat);

        statement.Finalize ();
        stat = statement.Prepare (ecdb, verifySelectECSql);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of verification ECSQL '" << verifySelectECSql << "' failed: " << statement.GetLastStatusMessage ();
        statement.BindId (1, ecInstanceKey.GetECInstanceId ());

        stepStat = statement.Step ();
        ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stepStat);

        IECSqlValue const& structValue = statement.GetValue (structValueIndex);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());
        VerifyECSqlValue (statement, expectedStructValue, structValue);
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

    testFunction ("INSERT INTO ecsql.PSA (I, PStructProp) VALUES (?, ?)", true, 2, structValueJson, "SELECT I, PStructProp FROM ecsql.PSA WHERE ECInstanceId = ?", 1);

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

    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", true, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    //**** Type mismatch tests *****
    //SQLite primitives are compatible to each other (test framework does not allow that yet)
    /*structValueJson = "{\"PStructProp\" : {\"bi\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", true, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"s\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", true, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);
    */
    //ECSQL primitives PointXD and DateTime are only compatible with themselves.
    structValueJson = "{\"PStructProp\" : {\"p2d\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"p3d\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"dt\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"dtUtc\" : 3.1415 }}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);

    structValueJson = "{\"PStructProp\" : {\"dtUtc\" : {"
                                    "\"type\" : \"datetime\","
                                    "\"datetime\" : \"2014-03-27T13:14:00.000\"}}}";
    testFunction ("INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", false, 1, structValueJson, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 04/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_UpdateWithStructBinding)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    //insert some test instances
    auto insertFunction = [this, &ecdb] (ECInstanceKey& ecInstanceKey, Utf8CP insertECSql, int structParameterIndex, Utf8CP structValueToBindJson)
        {
        Json::Value structValue (Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse (structValueToBindJson, structValue);
        ASSERT_TRUE (parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, insertECSql);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of ECSQL '" << insertECSql << "' failed: " << statement.GetLastStatusMessage ();

        IECSqlBinder& structBinder = statement.GetBinder (structParameterIndex);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson (bindStatus, statement, structValue, structBinder);

        auto stepStat = statement.Step (ecInstanceKey);
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStat) << "Execution of ECSQL '" << insertECSql << "' failed: " << statement.GetLastStatusMessage ();
        };

      auto testFunction = [this, &ecdb] (Utf8CP updateECSql, int structParameterIndex, Utf8CP structValueJson, int ecInstanceIdParameterIndex, ECInstanceKey ecInstanceKey, Utf8CP verifySelectECSql, int structValueIndex)
        {
        Json::Value expectedStructValue (Json::objectValue);
        bool parseSucceeded = Json::Reader::Parse (structValueJson, expectedStructValue);
        ASSERT_TRUE (parseSucceeded);

        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, updateECSql);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of ECSQL '" << updateECSql << "' failed: " << statement.GetLastStatusMessage ();

        stat = statement.BindId (ecInstanceIdParameterIndex, ecInstanceKey.GetECInstanceId ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Binding ECInstanceId to ECSQL '" << updateECSql << "' failed: " << statement.GetLastStatusMessage ();

        auto& binder = statement.GetBinder (structParameterIndex);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());

        BentleyStatus bindStatus = SUCCESS;
        BindFromJson (bindStatus, statement, expectedStructValue, binder);

        statement.EnableDefaultEventHandler ();
        auto stepStat = statement.Step ();

        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStat);
        ASSERT_EQ(1, statement.GetDefaultEventHandler ()->GetInstancesAffectedCount());

        statement.Finalize ();
        stat = statement.Prepare (ecdb, verifySelectECSql);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of verification ECSQL '" << verifySelectECSql << "' failed: " << statement.GetLastStatusMessage ();
        statement.BindId (1, ecInstanceKey.GetECInstanceId ());

        stepStat = statement.Step ();
        ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stepStat);

        IECSqlValue const& structValue = statement.GetValue (structValueIndex);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ());
        VerifyECSqlValue (statement, expectedStructValue, structValue);
        };

    //Insert test instances
    ECInstanceKey psaECInstanceKey;
    insertFunction (psaECInstanceKey, "INSERT INTO ecsql.PSA (PStructProp) VALUES (?)", 1,
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
    insertFunction (saECInstanceKey, "INSERT INTO ecsql.SA (SAStructProp) VALUES (?)", 1,
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

    testFunction ("UPDATE ONLY ecsql.PSA SET PStructProp = ? WHERE ECInstanceId = ?", 1, newStructValueJson, 2, psaECInstanceKey, "SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId = ?", 0);

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

    testFunction ("UPDATE ONLY ecsql.SA SET SAStructProp = ? WHERE ECInstanceId = ?", 1, newStructValueJson, 2, saECInstanceKey, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId = ?", 0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_ParameterInSelectClause)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::Readonly), perClassRowCount);

        {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ?, S FROM ecsql.PSA LIMIT 1")) << statement.GetLastStatusMessage().c_str();

        ECInstanceId expectedId(BeRepositoryId(3), 444);
        ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expectedId)) << statement.GetLastStatusMessage().c_str();

        ASSERT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        ASSERT_EQ(expectedId.GetValue(), statement.GetValueId<ECInstanceId>(0).GetValueUnchecked());
        ASSERT_EQ(ECSqlStepStatus::Done, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        ASSERT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        ASSERT_TRUE(statement.IsValueNull(0));
        ASSERT_EQ(ECSqlStepStatus::Done, statement.Step());
        }

        {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT -?, S FROM ecsql.PSA LIMIT 1")) << statement.GetLastStatusMessage().c_str();

        ECInstanceId expectedId(BeRepositoryId(3), 444);
        ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expectedId)) << statement.GetLastStatusMessage().c_str();

        ASSERT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        ASSERT_EQ((-1)*expectedId.GetValue(), statement.GetValueId<ECInstanceId>(0).GetValueUnchecked());
        ASSERT_EQ(ECSqlStepStatus::Done, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        ASSERT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        ASSERT_TRUE(statement.IsValueNull(0));
        ASSERT_EQ(ECSqlStepStatus::Done, statement.Step());
        }

        {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT -? AS MyId, S FROM ecsql.PSA LIMIT 1")) << statement.GetLastStatusMessage().c_str();

        int64_t expectedId = -123456LL;
        ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, expectedId)) << statement.GetLastStatusMessage().c_str();

        ASSERT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        ASSERT_EQ((-1)*expectedId, statement.GetValueInt64(0));
        ASSERT_EQ(ECSqlStepStatus::Done, statement.Step());

        statement.Reset();
        statement.ClearBindings();
        ASSERT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        ASSERT_TRUE(statement.IsValueNull(0));
        ASSERT_EQ(ECSqlStepStatus::Done, statement.Step());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_GetParameterIndex)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :i AND S = :s AND L = :i * 1000000 + 456789");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        int actualParamIndex = statement.GetParameterIndex ("i");
        EXPECT_EQ (1, actualParamIndex);
        statement.BindInt (actualParamIndex, 123);

        actualParamIndex = statement.GetParameterIndex ("s");
        EXPECT_EQ (2, actualParamIndex);
        statement.BindText (actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

        actualParamIndex = statement.GetParameterIndex ("garbage");
        EXPECT_EQ (-1, actualParamIndex);

        auto stepStat = statement.Step ();
        EXPECT_EQ ((int) ECSqlStepStatus::HasRow, (int) stepStat);
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.BindInt (1, 123);

        int actualParamIndex = statement.GetParameterIndex ("s");
        EXPECT_EQ (2, actualParamIndex);
        statement.BindText (actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

        actualParamIndex = statement.GetParameterIndex ("l");
        EXPECT_EQ (3, actualParamIndex);
        statement.BindInt64 (actualParamIndex, 123456789);

        auto stepStat = statement.Step ();
        EXPECT_EQ ((int) ECSqlStepStatus::HasRow, (int) stepStat);
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = ?");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.BindInt (1, 123);

        int actualParamIndex = statement.GetParameterIndex ("s");
        EXPECT_EQ (2, actualParamIndex);
        statement.BindText (actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

        statement.BindInt64 (3, 123456789);

        auto stepStat = statement.Step ();
        EXPECT_EQ ((int) ECSqlStepStatus::HasRow, (int) stepStat);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertColumnInfo (WCharCP expectedPropertyName, bool expectedIsGenerated, Utf8CP expectedPropPathStr, WCharCP expectedRootClassName, Utf8CP expectedRootClassAlias,
                       ECSqlColumnInfoCR actualColumnInfo)
    {
    auto actualProperty = actualColumnInfo.GetProperty ();
    if (expectedPropertyName == nullptr)
        {
        EXPECT_TRUE (actualProperty == nullptr);
        }
    else
        {
        ASSERT_TRUE (actualProperty != nullptr);
        EXPECT_STREQ (expectedPropertyName, actualProperty->GetName ().c_str ());
        }

    EXPECT_EQ (expectedIsGenerated, actualColumnInfo.IsGeneratedProperty ());

    auto const& actualPropPath = actualColumnInfo.GetPropertyPath ();
    Utf8String actualPropPathStr = actualPropPath.ToString ();
    EXPECT_STREQ (expectedPropPathStr, actualPropPathStr.c_str ());
    LOG.tracev ("Property path: %s", actualPropPathStr.c_str ());

    EXPECT_STREQ (expectedRootClassName, actualColumnInfo.GetRootClass ().GetName ().c_str ());
    if (expectedRootClassAlias == nullptr)
        EXPECT_TRUE (Utf8String::IsNullOrEmpty (actualColumnInfo.GetRootClassAlias ()));
    else
        EXPECT_STREQ (expectedRootClassAlias, actualColumnInfo.GetRootClassAlias ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ColumnInfoForPrimitiveArrays)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

    ECSqlStatement stmt;
    auto stat = stmt.Prepare (ecdb, "SELECT c.Dt_Array FROM ecsql.PSA c LIMIT 1");
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));

    auto stepStat = stmt.Step ();
    ASSERT_EQ (static_cast<int> (ECSqlStepStatus::HasRow), static_cast<int> (stepStat));

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo (0);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"Dt_Array", false, "Dt_Array", L"PSA", "c", topLevelColumnInfo);
    auto const& topLevelArrayValue = stmt.GetValueArray (0);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));

    //out of bounds test
    BeTest::SetFailOnAssert (false);
        {
        stmt.GetColumnInfo (-1);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after ECSqlStatement::GetColumnInfo (-1)";
        stmt.GetColumnInfo (2);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after ECSqlStatement::GetColumnInfo with too large index";
        }
    BeTest::SetFailOnAssert (true);

    //In array level
    int arrayIndex = 0;
    for (IECSqlValue const* arrayElement : topLevelArrayValue)
        {
        auto const& arrayElementColumnInfo = arrayElement->GetColumnInfo ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "Primitive array element IECSqlValue::GetColumnInfo failed.";
        Utf8String expectedPropPath;
        expectedPropPath.Sprintf ("Dt_Array[%d]", arrayIndex);
        AssertColumnInfo (nullptr, false, expectedPropPath.c_str (), L"PSA", "c", arrayElementColumnInfo);

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ColumnInfoForStructs)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

    ECSqlStatement stmt;
    auto stat = stmt.Prepare (ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1");
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));

    auto stepStat = stmt.Step ();
    ASSERT_EQ (static_cast<int> (ECSqlStepStatus::HasRow), static_cast<int> (stepStat));

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo (0);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"SAStructProp", false, "SAStructProp", L"SA", nullptr, topLevelColumnInfo);

    //out of bounds test
    BeTest::SetFailOnAssert (false);
        {
        stmt.GetColumnInfo (-1);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after ECSqlStatement::GetColumnInfo (-1)";
        stmt.GetColumnInfo (2);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after ECSqlStatement::GetColumnInfo with too large index";
        }
    BeTest::SetFailOnAssert (true);

    //SAStructProp.PStructProp level
    auto const& topLevelStructValue = stmt.GetValueStruct (0);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    auto const& nestedStructPropColumnInfo = topLevelStructValue.GetValue (0).GetColumnInfo (); //0 refers to first member in SAStructProp which is PStructProp
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "Struct IECSqlValue::GetColumnInfo ()";
    AssertColumnInfo (L"PStructProp", false, "SAStructProp.PStructProp", L"SA", nullptr, nestedStructPropColumnInfo);
    auto const& nestedStructValue = topLevelStructValue.GetValue (0).GetStruct ();
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "Struct IECSqlValue::GetStruct ()";;

    //out of bounds test
    BeTest::SetFailOnAssert (false);
        {
        topLevelStructValue.GetValue (-1);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue (-1) for struct value.";
        topLevelStructValue.GetValue (topLevelStructValue.GetMemberCount ());
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue with too large index for struct value";
        }
    BeTest::SetFailOnAssert (true);

    //SAStructProp.PStructProp.XXX level
    auto const& firstStructMemberColumnInfo = nestedStructValue.GetValue (0).GetColumnInfo ();
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"b", false, "SAStructProp.PStructProp.b", L"SA", nullptr, firstStructMemberColumnInfo);

    auto const& secondStructMemberColumnInfo = nestedStructValue.GetValue (1).GetColumnInfo ();
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"bi", false, "SAStructProp.PStructProp.bi", L"SA", nullptr, secondStructMemberColumnInfo);

    auto const& eighthStructMemberColumnInfo = nestedStructValue.GetValue (8).GetColumnInfo ();
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"p2d", false, "SAStructProp.PStructProp.p2d", L"SA", nullptr, eighthStructMemberColumnInfo);

    //out of bounds test
    BeTest::SetFailOnAssert (false);
        {
        nestedStructValue.GetValue (-1).GetColumnInfo ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue (-1) for struct value on second nesting level.";
        nestedStructValue.GetValue (nestedStructValue.GetMemberCount ());
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue with too large index for struct value on second nesting level.";
        }
    BeTest::SetFailOnAssert (true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ColumnInfoForStructArrays)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

    ECSqlStatement stmt;
    auto stat = stmt.Prepare (ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1");
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));

    auto stepStat = stmt.Step ();
    ASSERT_EQ (static_cast<int> (ECSqlStepStatus::HasRow), static_cast<int> (stepStat));

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo (0);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"SAStructProp", false, "SAStructProp", L"SA", nullptr, topLevelColumnInfo);
    auto const& topLevelStructValue = stmt.GetValueStruct (0);
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    
    //out of bounds test
    BeTest::SetFailOnAssert (false);
        {
        stmt.GetColumnInfo (-1);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after ECSqlStatement::GetColumnInfo (-1)";
        stmt.GetColumnInfo (2);
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after ECSqlStatement::GetColumnInfo with too large index";
        }
    BeTest::SetFailOnAssert (true);

    //SAStructProp.PStruct_Array level
    int columnIndex = 1;
    auto const& pstructArrayColumnInfo = topLevelStructValue.GetValue (columnIndex).GetColumnInfo ();
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
    AssertColumnInfo (L"PStruct_Array", false, "SAStructProp.PStruct_Array", L"SA", nullptr, pstructArrayColumnInfo);
    auto const& pstructArrayValue = topLevelStructValue.GetValue (columnIndex).GetArray ();
    ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));

    //out of bounds test
    BeTest::SetFailOnAssert (false);
        {
        topLevelStructValue.GetValue (-1).GetColumnInfo ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue (-1).GetColumnInfo () for struct value";
        topLevelStructValue.GetValue (topLevelStructValue.GetMemberCount ()).GetColumnInfo ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue (N).GetColumnInfo with N being too large index for struct value";
        }
    BeTest::SetFailOnAssert (true);

    //SAStructProp.PStruct_Array[] level
    int arrayIndex = 0;
    Utf8String expectedPropPath;
    for (IECSqlValue const* arrayElement : pstructArrayValue)
        {
        IECSqlStructValue const& pstructArrayElement = arrayElement->GetStruct ();
        //first struct member
        auto const& arrayElementFirstColumnInfo = pstructArrayElement.GetValue (0).GetColumnInfo ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));

        expectedPropPath.Sprintf ("SAStructProp.PStruct_Array[%d].b", arrayIndex);
        AssertColumnInfo (L"b", false, expectedPropPath.c_str (), L"SA", nullptr, arrayElementFirstColumnInfo);

        //second struct member
        auto const& arrayElementSecondColumnInfo = pstructArrayElement.GetValue (1).GetColumnInfo ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
        expectedPropPath.Sprintf ("SAStructProp.PStruct_Array[%d].bi", arrayIndex);
        AssertColumnInfo (L"bi", false, expectedPropPath.c_str (), L"SA", nullptr, arrayElementSecondColumnInfo);

        //out of bounds test
        BeTest::SetFailOnAssert (false);
            {
            pstructArrayElement.GetValue (-1).GetColumnInfo ();
            ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue (-1).GetColumnInfo () for struct array value";
            pstructArrayElement.GetValue (pstructArrayElement.GetMemberCount ()).GetColumnInfo ();
            ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after GetValue (N).GetColumnInfo with N being too large index for struct array value";
            }
        BeTest::SetFailOnAssert (true);

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_Step)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "SELECT * FROM ecsql.P");
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

        auto stepStat = statement.Step ();
        ASSERT_EQ ((int)ECSqlStepStatus::HasRow, (int)stepStat) << "Step() on ECSQL SELECT statement is expected to be allowed.";
        }

        {
        ECSqlStatement statement;
        ECInstanceKey ecInstanceKey;
        auto stepStat = statement.Step (ecInstanceKey);
        ASSERT_EQ ((int)ECSqlStepStatus::Error, (int)stepStat) << "Step(ECInstanceKey&) on ECSQL SELECT statement is expected to not be allowed.";
        }

        {
        ECSqlStatement statement;
        auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.P (I, L) VALUES (100, 10203)");
        ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

        auto stepStat = statement.Step ();
        ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStat) << "Step() on ECSQL INSERT statement is expected to be allowed.";

        statement.Reset ();
        ECInstanceKey ecInstanceKey;
        stepStat = statement.Step (ecInstanceKey);
        ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStat) << "Step(ECInstanceKey&) on ECSQL INSERT statement is expected to be allowed.";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_MultipleInsertsWithoutReprepare)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (?, ?)");
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    int firstIntVal = 1;
    Utf8CP firstStringVal = "First insert";
    stat = statement.BindInt (1, firstIntVal);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);
    stat = statement.BindText (2, firstStringVal, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    ECInstanceKey firstECInstanceKey;
    auto stepStat = statement.Step (firstECInstanceKey);
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStat) << statement.GetLastStatusMessage ().c_str ();

    stat = statement.Reset ();
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);
    stat = statement.ClearBindings ();
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    //second insert with same statement

    int secondIntVal = 2;
    Utf8CP secondStringVal = "Second insert";
    stat = statement.BindInt (1, secondIntVal);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);
    stat = statement.BindText (2, secondStringVal, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    ECInstanceKey secondECInstanceKey;
    stepStat = statement.Step (secondECInstanceKey);
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStat);

    //check the inserts
    ASSERT_GT (secondECInstanceKey.GetECInstanceId ().GetValue (), firstECInstanceKey.GetECInstanceId ().GetValue ());

    statement.Finalize ();
    stat = statement.Prepare (ecdb, "SELECT ECInstanceId, I, S FROM ecsql.PSA WHERE ECInstanceId = ?");
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);
    
    //check first insert
    stat = statement.BindId (1, firstECInstanceKey.GetECInstanceId ());
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    stepStat = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::HasRow, (int)stepStat);
    ASSERT_EQ (firstECInstanceKey.GetECInstanceId ().GetValue (), statement.GetValueId<ECInstanceId> (0).GetValue ());
    ASSERT_EQ (firstIntVal, statement.GetValueInt (1));
    ASSERT_STREQ (firstStringVal, statement.GetValueText (2));
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)statement.Step ());

    //check second insert
    stat = statement.Reset ();
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);
    stat = statement.ClearBindings ();
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    stat = statement.BindId (1, secondECInstanceKey.GetECInstanceId ());
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat);

    stepStat = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::HasRow, (int)stepStat);
    ASSERT_EQ (secondECInstanceKey.GetECInstanceId ().GetValue (), statement.GetValueId<ECInstanceId> (0).GetValue ());
    ASSERT_EQ (secondIntVal, statement.GetValueInt (1));
    ASSERT_STREQ (secondStringVal, statement.GetValueText (2));
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)statement.Step ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_Reset)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

        {
        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, "SELECT * FROM ecsql.P LIMIT 2");
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation for a valid ECSQL failed.";
        int expectedRowCount = 0;
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            expectedRowCount++;
            }

        stat = stmt.Reset ();
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "After ECSqlStatement::Reset";
        int actualRowCount = 0;
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            actualRowCount++;
            }
        ASSERT_EQ (expectedRowCount, actualRowCount) << "After resetting ECSqlStatement is expected to return same number of returns as after preparation.";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_Finalize)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

        {
        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, "SELECT * FROM blablabla");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::InvalidECSql), static_cast<int> (stat)) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

        stmt.Finalize ();
        stat = stmt.Prepare (ecdb, "SELECT * FROM ecsql.P");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "Preparation for a valid ECSQL failed.";
        int actualRowCount = 0;
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            actualRowCount++;
            }

        ASSERT_EQ (perClassRowCount, actualRowCount);
        }

        {
        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, "SELECT * FROM ecsql.P");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "Preparation for a valid ECSQL failed.";

        int actualRowCount = 0;
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            actualRowCount++;
            }
        ASSERT_EQ (perClassRowCount, actualRowCount);

        //now finalize and do the exact same stuff. In particular this tests that the cursor is reset so that we get all results
        stmt.Finalize ();
        stat = stmt.Prepare (ecdb, "SELECT * FROM ecsql.P");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "Preparation for a valid ECSQL failed.";
        actualRowCount = 0;
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            actualRowCount++;
            }

        ASSERT_EQ (perClassRowCount, actualRowCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_LastStatus)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::Readonly), perClassRowCount);

        {
        ECSqlStatement stmt;
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus on a new ECSqlStatement is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage on a new ECSqlStatement is expected to return empty string.";

        auto stat = stmt.Prepare (ecdb, "SELECT * FROM ecsql.P WHERE I = ?");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "Preparation for a valid ECSQL failed.";
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Prepare is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to Prepare is expected to return empty string.";

        BeTest::SetFailOnAssert (false);
        stat = stmt.BindPoint2D (1, DPoint2d::From (1.0, 1.0));
        BeTest::SetFailOnAssert (true);
        ASSERT_EQ (static_cast<int> (stat), static_cast<int> (stmt.GetLastStatus ()));
        ASSERT_EQ (static_cast<int> (ECSqlStatus::UserError), static_cast<int> (stat)) << "Cannot bind points to int parameter";

        stat = stmt.BindInt (1, 123);
        ASSERT_EQ (static_cast<int> (stat), static_cast<int> (stmt.GetLastStatus ()));
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat));

        BeTest::SetFailOnAssert (false);
        stat = stmt.BindDouble (2, 3.14);
        BeTest::SetFailOnAssert (true);
        ASSERT_EQ (static_cast<int> (stat), static_cast<int> (stmt.GetLastStatus ()));
        ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stat)) << "Index out of bounds error expected.";

        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Step is expected to return ECSqlStatus::Success.";
            ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to Step is expected to return empty string.";

            stmt.GetColumnInfo (0);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
            
            BeTest::SetFailOnAssert (false);
            stmt.GetColumnInfo (-1);
            BeTest::SetFailOnAssert (true);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ()));
            stmt.GetColumnInfo (0);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
            BeTest::SetFailOnAssert (false);
            stmt.GetColumnInfo (100);
            BeTest::SetFailOnAssert (true);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ()));
            stmt.GetColumnInfo (0);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));

            BeTest::SetFailOnAssert (false);
            stmt.GetValueInt (-1);
            BeTest::SetFailOnAssert (true);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ()));
            stmt.GetValueInt (0);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
            BeTest::SetFailOnAssert (false);
            stmt.GetValueInt (100);
            BeTest::SetFailOnAssert (true);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::IndexOutOfBounds), static_cast<int> (stmt.GetLastStatus ()));
            stmt.GetValueDouble (1);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ()));
            BeTest::SetFailOnAssert (false);
            stmt.GetValuePoint2D (1);
            BeTest::SetFailOnAssert (true);
            ASSERT_EQ (static_cast<int> (ECSqlStatus::UserError), static_cast<int> (stmt.GetLastStatus ()));
            }

        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Step is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to Step is expected to return empty string.";

        stat = stmt.ClearBindings ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "ECSqlStatement::Reset failed unexpectedly.";
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to ClearBindings is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to ClearBindings is expected to return empty string.";

        stat = stmt.Reset ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "ECSqlStatement::Reset failed unexpectedly.";
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Reset is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after successful call to Reset is expected to return empty string.";

        stmt.Finalize ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Finalize is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "ECSqlStatement::GetLastStatusMessage after call to Finalize is expected to return empty string.";
        }

        {
        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, "SELECT * FROM blablabla");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::InvalidECSql), static_cast<int> (stat)) << "Preparation for an invalid ECSQL succeeded unexpectedly.";
        
        auto actualLastStatusMessage = stmt.GetLastStatusMessage ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::InvalidECSql), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after preparing invalid ECSQL.";
        ASSERT_STREQ (actualLastStatusMessage.c_str (), "Invalid ECSQL 'SELECT * FROM blablabla': ECClass 'blablabla' does not exist. Try using fully qualified class name: <schema name>.<class name>.");

        stmt.Finalize ();
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after call to Finalize is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after call to Finalize is expected to return empty string.";

        //now reprepare with valid ECSQL
        stat = stmt.Prepare (ecdb, "SELECT * FROM ecsql.P");
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stat)) << "Preparation for a valid ECSQL failed.";
        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Prepare is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to Prepare is expected to return empty string.";

        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Step is expected to return ECSqlStatus::Success.";
            ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to Step is expected to return empty string.";
            }

        ASSERT_EQ (static_cast<int> (ECSqlStatus::Success), static_cast<int> (stmt.GetLastStatus ())) << "GetLastStatus after successful call to Step is expected to return ECSqlStatus::Success.";
        ASSERT_TRUE (stmt.GetLastStatusMessage ().empty ()) << "GetLastStatusMessage after successful call to Step is expected to return empty string.";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertGeometry (IGeometryCR expected, IGeometryCR actual, Utf8P assertMessage)
    {
    ASSERT_TRUE (actual.IsSameStructureAndGeometry (expected)) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_Geometry)
    {
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), 0);

    std::vector<IGeometryPtr> expectedGeoms;
        
    IGeometryPtr line1 = IGeometry::Create (ICurvePrimitive::CreateLine (DSegment3d::From (0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr line2 = IGeometry::Create (ICurvePrimitive::CreateLine (DSegment3d::From (1.0, 1.0, 1.0, 2.0, 2.0, 2.0)));
    IGeometryPtr line3 = IGeometry::Create (ICurvePrimitive::CreateLine (DSegment3d::From (2.0, 2.0, 2.0, 3.0, 3.0, 3.0)));

    expectedGeoms.push_back(line1);
    expectedGeoms.push_back(line2);
    expectedGeoms.push_back(line3);
    IGeometryPtr expectedGeomSingle = expectedGeoms[0];

    // insert geometries in various variations
        {
        auto ecsql = "INSERT INTO ecsql.PASpatial (Geometry, B, Geometry_Array) VALUES(?, True, ?)";

        ECSqlStatement statement;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.Prepare (ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindGeometry (1, *expectedGeomSingle));

        IECSqlArrayBinder& arrayBinder = statement.BindArray (2, 3);
        for (auto& geom : expectedGeoms)
            {
            auto& arrayElementBinder = arrayBinder.AddArrayElement ();
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "IECSqlArrayBinder::AddArrayElement is expected to succeed";
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) arrayElementBinder.BindGeometry (*geom));
            }

        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step ());
        }

        {
        auto ecsql = "INSERT INTO ecsql.SSpatial (PASpatialProp.Geometry, PASpatialProp.Geometry_Array) VALUES(?,?)";

        ECSqlStatement statement;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.Prepare (ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
        
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.BindGeometry (1, *expectedGeomSingle));

        IECSqlArrayBinder& arrayBinder = statement.BindArray (2, 3);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "ECSqlStatement::BindArray is expected to succeed";
        for (auto& geom : expectedGeoms)
            {
            auto& arrayElementBinder = arrayBinder.AddArrayElement ();
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "IECSqlArrayBinder::AddArrayElement is expected to succeed";
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) arrayElementBinder.BindGeometry (*geom));
            }

        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step ());
        }

        {
        auto ecsql = "INSERT INTO ecsql.SSpatial (PASpatialProp) VALUES(?)";

        ECSqlStatement statement;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.Prepare (ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

        IECSqlStructBinder& structBinder = statement.BindStruct (1);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "ECSqlStatement::BindStruct is expected to succeed";

        ASSERT_EQ ((int) ECSqlStatus::Success, (int) structBinder.GetMember (L"Geometry").BindGeometry (*expectedGeomSingle));

        IECSqlArrayBinder& arrayBinder = structBinder.GetMember (L"Geometry_Array").BindArray (3);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "IECSqlBinder::BindArray is expected to succeed";
        for (auto& geom : expectedGeoms)
            {
            auto& arrayElementBinder = arrayBinder.AddArrayElement ();
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.GetLastStatus ()) << "IECSqlArrayBinder::AddArrayElement is expected to succeed";
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) arrayElementBinder.BindGeometry (*geom));
            }

        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step ());
        }

    ecdb.SaveChanges ();
    ecdb.ClearECDbCache();

    //now verify the inserts
    ECSqlStatement statement;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.Prepare (ecdb, "SELECT B, Geometry_Array, Geometry FROM ecsql.PASpatial")) << "Preparation failed: " << statement.GetLastStatusMessage ();
    int rowCount = 0;
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        rowCount++;

        ASSERT_TRUE (statement.GetValueBoolean (0)) << "First column value is expected to be true";

        IECSqlArrayValue const& arrayVal = statement.GetValueArray (1);
        int i = 0;
        for (IECSqlValue const* arrayElem : arrayVal)
            {
            IGeometryPtr actualGeom = arrayElem->GetGeometry ();
            ASSERT_TRUE (actualGeom != nullptr);

            AssertGeometry (*expectedGeoms[i], *actualGeom, "PASpatial.Geometry_Array");
            i++;
            }
        ASSERT_EQ ((int) expectedGeoms.size (), i);

        IGeometryPtr actualGeom = statement.GetValueGeometry (2);
        ASSERT_TRUE (actualGeom != nullptr);
        AssertGeometry (*expectedGeomSingle, *actualGeom, "PASpatial.Geometry");
        }
    ASSERT_EQ (1, rowCount);

    statement.Finalize ();
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.Prepare (ecdb, "SELECT PASpatialProp.Geometry_Array, PASpatialProp.Geometry FROM ecsql.SSpatial")) << "Preparation failed: " << statement.GetLastStatusMessage ();
    rowCount = 0;
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        rowCount++;

        IECSqlArrayValue const& arrayVal = statement.GetValueArray (0);
        int i = 0;
        for (IECSqlValue const* arrayElem : arrayVal)
            {
            IGeometryPtr actualGeom = arrayElem->GetGeometry ();
            ASSERT_TRUE (actualGeom != nullptr);

            AssertGeometry (*expectedGeoms[i], *actualGeom, "SSpatial.PASpatialProp.Geometry_Array");
            i++;
            }
        ASSERT_EQ ((int) expectedGeoms.size (), i);

        IGeometryPtr actualGeom = statement.GetValueGeometry (1);
        ASSERT_TRUE (actualGeom != nullptr);
        AssertGeometry (*expectedGeomSingle, *actualGeom, "SSpatial.PASpatialProp.Geometry");
        }
    ASSERT_EQ (2, rowCount);

    statement.Finalize ();
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.Prepare (ecdb, "SELECT PASpatialProp FROM ecsql.SSpatial")) << "Preparation failed: " << statement.GetLastStatusMessage ();
    rowCount = 0;
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        rowCount++;

        IECSqlStructValue const& structVal = statement.GetValueStruct (0);
        for (int i = 0; i < structVal.GetMemberCount (); i++)
            {
            IECSqlValue const& structMemberVal = structVal.GetValue (0);
            WStringCR structMemberName = structMemberVal.GetColumnInfo ().GetProperty ()->GetName ();
            if (structMemberName.Equals (L"Geometry"))
                {
                IGeometryPtr actualGeom = structMemberVal.GetGeometry ();
                AssertGeometry (*expectedGeomSingle, *actualGeom, "SSpatial.PASpatialProp > Geometry");
                }
            else if (structMemberName.Equals (L"Geometry_Array"))
                {
                IECSqlArrayValue const& arrayVal = structMemberVal.GetArray ();
                int i = 0;
                for (IECSqlValue const* arrayElem : arrayVal)
                    {
                    IGeometryPtr actualGeom = arrayElem->GetGeometry ();
                    ASSERT_TRUE (actualGeom != nullptr);

                    AssertGeometry (*expectedGeoms[i], *actualGeom, "SSpatial.PASpatialProp > Geometry_Array");
                    i++;
                    }
                ASSERT_EQ ((int) expectedGeoms.size (), i);
                }
            }
        }
    ASSERT_EQ (2, rowCount);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_GetGeometryWithInvalidBlobFormat)
    {
    // Create sample project without populating rows
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), 0);

    // insert invalid geom blob
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "INSERT INTO ecsqltest_ArrayOfPASpatial (ECInstanceId, Geometry) VALUES (1,?)"));
    double dummyValue = 3.141516;
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindBlob(1, &dummyValue, (int) sizeof(dummyValue), Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(ecdb, "SELECT Geometry FROM ecsql.PASpatial"));
    int rowCount = 0;
    while (ecsqlStmt.Step() == ECSqlStepStatus::HasRow)
        {
        rowCount++;
        ASSERT_FALSE(ecsqlStmt.IsValueNull(0)) << "Geometry column is not expected to be null.";

        ASSERT_TRUE(ecsqlStmt.GetValueGeometry(0) == nullptr) << "Invalid geom blob format expected so that nullptr is returned.";
        ASSERT_EQ(ECSqlStatus::UserError, ecsqlStmt.GetLastStatus());
        }

    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayInsert)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp) VALUES(?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    IECSqlStructBinder& saStructBinder = statement.BindStruct(1); //SAStructProp
    ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "AddArrayElement failed: " << statement.GetLastStatusMessage();
    IECSqlStructBinder& pStructBinder = saStructBinder.GetMember(L"PStructProp").BindStruct();
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
    stat = pStructBinder.GetMember(L"i").BindInt(99);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();

    //add three array elements
    const int count = 3;
    auto& arrayBinder = saStructBinder.GetMember(L"PStruct_Array").BindArray((uint32_t)count);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "BindArray failed: " << statement.GetLastStatusMessage();
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "AddArrayElement failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"d").BindDouble(i * PI);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"i").BindInt(i * 2);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"l").BindInt64(i * 3);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember(L"p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        auto &pStructArray = stmt.GetValue(0).GetArray();
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_StructArrayInsertWithParametersLongAndArray)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(123, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

   //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray(1, (uint32_t)count);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "BindArray failed: " << statement.GetLastStatusMessage();
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "AddArrayElement failed: " << statement.GetLastStatusMessage();
        auto stat = structBinder.GetMember(L"d").BindDouble(i * PI);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"i").BindInt(i * 2);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"l").BindInt64(i * 3);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember(L"p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, L"L");
        ASSERT_EQ(123,v.GetLong());
        inst->GetValue(v, L"PStruct_Array");
        ASSERT_EQ(count, v.GetArrayInfo().GetCount());
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_InsertWithMixParametersIntAndInt)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.Sub1 (I,Sub1I) VALUES(123, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

    statement.BindInt(1, 333);

    auto stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, L"I");
        ASSERT_EQ(123, v.GetInteger());
        inst->GetValue(v, L"Sub1I");
        ASSERT_EQ(123, v.GetInteger());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_InsertWithMixParameters)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.P (B,D,I,L,S) VALUES(1, ?,?,123,?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

    statement.BindDouble(1, 2.22);
    statement.BindInt(2, 123);
    statement.BindText(3, "Test Test",IECSqlBinder::MakeCopy::Yes);
    auto stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, L"B");
        ASSERT_EQ(true, v.GetBoolean());
        inst->GetValue(v, L"D");
        ASSERT_EQ(2.22, v.GetDouble());
        inst->GetValue(v, L"I");
        ASSERT_EQ(123, v.GetInteger());
        inst->GetValue(v, L"L");
        ASSERT_EQ(123, v.GetLong());
        inst->GetValue(v, L"S");
        ASSERT_STREQ(L"Test Test", v.GetString());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayInsertWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray(1,(uint32_t)count);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "BindArray failed: " << statement.GetLastStatusMessage();
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "AddArrayElement failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"d").BindDouble(i * PI);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"i").BindInt(i * 2);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"l").BindInt64(i * 3);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember(L"p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (stmt.Step() == ECSqlStepStatus::HasRow)
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

        // ASSERT_TRUE(ECOBJECTS_STATUS_Success == inst->GetValue(v, L"SAStructProp.PStruct_Array",0));
        // IECInstancePtr structInstance = v.GetStruct();
        // structInstance->GetValue(v, L"PStruct_Array");
        //ASSERT_TRUE(v.IsArray());
        ASSERT_TRUE(pStructArray == pStructArray);*/
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlTestFixture, ECSqlStatement_StructUpdateWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SAStruct (PStructProp.i) VALUES(2)";

    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    auto stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();
        {
        auto prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ecsql.SAStruct");
        ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
        ECInstanceECSqlSelectAdapter classPReader(statement);
        while (statement.Step() == ECSqlStepStatus::HasRow)
            {
            auto inst = classPReader.GetInstance();
            ECValue v;
            inst->GetValue(v, L"PStructProp.i");
            ASSERT_EQ(2, v.GetInteger());
            }
        }
    statement.Finalize();
    ecsql = "UPDATE  ONLY ecsql.SAStruct SET PStructProp.i = 3 ";
    stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();

    auto prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ecsql.SAStruct");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(statement);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, L"PStructProp.i");
        ASSERT_EQ(3, v.GetInteger());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                             Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayUpdateWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement insertStatement;
    auto stat = insertStatement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << insertStatement.GetLastStatusMessage();

    //add three array elements
    int count = 3;
    auto& arrayBinder = insertStatement.BindArray(1, (uint32_t)count);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)insertStatement.GetLastStatus()) << "BindArray failed: " << insertStatement.GetLastStatusMessage();
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_EQ((int)ECSqlStatus::Success, (int)insertStatement.GetLastStatus()) << "AddArrayElement failed: " << insertStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"d").BindDouble(i * PI);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << insertStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"i").BindInt(i * 2);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << insertStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"l").BindInt64(i * 3);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << insertStatement.GetLastStatusMessage();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember(L"p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << insertStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << insertStatement.GetLastStatusMessage();
        }

    auto stepStatus = insertStatement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << insertStatement.GetLastStatusMessage();
    ECSqlStatement selectStatement;
    auto prepareStatus = selectStatement.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (selectStatement.Step() == ECSqlStepStatus::HasRow)
        {
        auto &pStructArray = selectStatement.GetValue(0).GetArray();
        ASSERT_EQ(count, pStructArray.GetArrayLength());

        }
    ECSqlStatement updateStatement;
    ecsql = "UPDATE  ONLY ecsql.SA SET SAStructProp.PStruct_Array = ? ";
    prepareStatus = updateStatement.Prepare(ecdb, ecsql);
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    count = 3;
    auto& updateArrayBinder = updateStatement.BindArray(1, (uint32_t)count);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)updateStatement.GetLastStatus()) << "BindArray failed: " << updateStatement.GetLastStatusMessage();
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = updateArrayBinder.AddArrayElement().BindStruct();
        ASSERT_EQ((int)ECSqlStatus::Success, (int)updateStatement.GetLastStatus()) << "AddArrayElement failed: " << updateStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"d").BindDouble(-count );
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << updateStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"i").BindInt(-count );
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << updateStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"l").BindInt64(-count );
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << updateStatement.GetLastStatusMessage();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember(L"p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << updateStatement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << updateStatement.GetLastStatusMessage();
        }

    stepStatus = updateStatement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << updateStatement.GetLastStatusMessage();
   
    ECSqlStatement statement;
    prepareStatus = statement.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        auto &pStructArray = statement.GetValue(0).GetArray();
        ASSERT_EQ(count, pStructArray.GetArrayLength());
        }
    statement.Finalize();
    }

END_ECDBUNITTESTS_NAMESPACE
