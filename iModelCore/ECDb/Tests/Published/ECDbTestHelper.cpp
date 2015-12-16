/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbTestHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbTestHelper.h"
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementTests, Insert)
    {
        {
        ECTEST_SETUP ("Test1", "ECSqlStatementTests.01.00.ecschema.xml", L"Insert.ecdb");

        STATEMENT_PREPARE_SUCCESS ("INSERT INTO ECST.Product(ProductAvailable,ProductName,UnitPrice) VALUES(?,?,?)");
        BIND_BOOLEAN(1, true);
        BIND_TEXT(2, "Hoodie", IECSqlBinder::MakeCopy::No);
        BIND_DOUBLE (3, 350.56);
        STATEMENT_EXECUTE_SUCCESS();

        STATEMENT_PREPARE_SUCCESS("INSERT INTO ECST.Supplier(ContactName,Country) VALUES('John Snow','England')");
        STATEMENT_EXECUTE_SUCCESS ();

        STATEMENT_PREPARE ("INSERT INTO ECST.Orders(ShipCity) VALUES('NewYork')", ECSqlStatus::InvalidECSql);
        }

        {
        ECTEST_SETUP("Test2", "NestedStructArrayTest.01.00.ecschema.xml", L"Insert1.ecdb");

        STATEMENT_PREPARE_SUCCESS ("INSERT INTO nsat.ClassA(I,T) VALUES(8,'testVal')");
        STATEMENT_EXECUTE_SUCCESS ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementTests, Update)
    {
    ECTEST_SETUP ("Test3", "ECSqlStatementTests.01.00.ecschema.xml", L"Update.ecdb");

    STATEMENT_PREPARE_SUCCESS ("INSERT INTO ECST.Supplier(ContactName,Country) VALUES('John Snow','England')");
    STATEMENT_EXECUTE_SUCCESS ();

    STATEMENT_PREPARE ("UPDATE ECST.Supplier SET ContactName = 'Micheal' WHERE ContactName = 'John Snow'", ECSqlStatus::Success);
    STATEMENT_EXECUTE (DbResult::BE_SQLITE_DONE);

    STATEMENT_PREPARE ("UPDATE ECST.Supplier SET ContactName = 'Snape' WHERE ContactName = ?", ECSqlStatus::Success);
    BIND_TEXT(1, "Micheal", IECSqlBinder::MakeCopy::No);
    STATEMENT_EXECUTE (DbResult::BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementTests, Delete)
    {
    ECTEST_SETUP ("Test4", "NestedStructArrayTest.01.00.ecschema.xml", L"Delete.ecdb");

    STATEMENT_PREPARE ("INSERT INTO nsat.ClassA(I,T) VALUES(8,'testVal')", ECSqlStatus::Success);
    STATEMENT_EXECUTE (DbResult::BE_SQLITE_DONE);

    STATEMENT_PREPARE ("DELETE FROM nsat.ClassA WHERE I=?", ECSqlStatus::Success);
    BIND_INT(1,8);
    STATEMENT_EXECUTE (DbResult::BE_SQLITE_DONE);
    }

END_ECDBUNITTESTS_NAMESPACE