/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlSelectTests.cpp $
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
TEST (ECSqlSelectTests, Select)
    {
    ECTEST_SETUP ("ECSqlStatementTests", "ECSqlStatementTests.01.00.ecschema.xml", L"Select.ecdb");

    STATEMENT_PREPARE_SUCCESS ("INSERT INTO ECST.Supplier(ContactName,Country,Phone) VALUES('John Snow','England',12345)");
    STATEMENT_EXECUTE_SUCCESS ();
    STATEMENT_PREPARE_SUCCESS ("SELECT ContactName,Phone,Country FROM ECST.Supplier");
    ASSERT_STATEMENT_EXECUTE (DbResult::BE_SQLITE_ROW);
    ASSERT_TEXT (0, "John Snow");
    ASSERT_INT (1, 12345);
    ASSERT_TEXT (2, "England");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlSelectTests, CountWithDistinctClause)
    {
    ECTEST_SETUP ("CountWithDistinctClause", "ECSqlStatementTests.01.00.ecschema.xml", L"CountWithDistinctClause.ecdb");

    ADD_QUERY ("INSERT INTO ECST.Supplier(ContactName,Country,Phone) VALUES('Tom Hardy','France',012)");
    ADD_QUERY ("INSERT INTO ECST.Supplier(ContactName,Country,Phone) VALUES('John','UK',234)");
    ADD_QUERY ("INSERT INTO ECST.Supplier(ContactName,Country,Phone) VALUES('Snow','Spain',567)");
    ADD_QUERY ("INSERT INTO ECST.Supplier(ContactName,Country,Phone) VALUES('David','France',469)");
    ADD_QUERY ("INSERT INTO ECST.Supplier(ContactName,Country,Phone) VALUES('Beckham','Spain',7345)");
    EXECUTE_LIST ();

    STATEMENT_PREPARE_SUCCESS ("SELECT COUNT(DISTINCT Country) FROM ECST.Supplier");
    ASSERT_STATEMENT_EXECUTE (DbResult::BE_SQLITE_ROW);
    ASSERT_INT (0, 3); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlSelectTests, RelationalOperatorsOnPointProperties)
    {
    ECTEST_SETUP ("RelationalOperatorsOnPointProperties", "ECSqlTest.01.00.ecschema.xml", L"RelationalOperatorsOnPointProperties.ecdb");

    STATEMENT_PREPARE_SUCCESS ("INSERT INTO ecsql.P (I, P2D, P3D) VALUES (123, ?, ?)");
    BIND_POINT2D (1, DPoint2d::From (1, 2));
    BIND_POINT3D (2, DPoint3d::From (3, 4, 5));
    STATEMENT_EXECUTE_SUCCESS ();

    STATEMENT_PREPARE_SUCCESS ("INSERT INTO ecsql.P (I, P2D, P3D) VALUES (321, ?, ?)");
    BIND_POINT2D (1, DPoint2d::From (6, 7));
    BIND_POINT3D (2, DPoint3d::From (8, 9, 10));
    STATEMENT_EXECUTE_SUCCESS ();

    STATEMENT_PREPARE_SUCCESS ("INSERT INTO ecsql.P (I, P2D, P3D) VALUES (456, ?, ?)");
    BIND_POINT2D (1, DPoint2d::From (12.5, 14.5));
    BIND_POINT3D (2, DPoint3d::From (9.5, 10.5, 11.5));
    STATEMENT_EXECUTE_SUCCESS ();

    STATEMENT_PREPARE_SUCCESS ("SELECT GetX(P2D) FROM ecsql.P WHERE GetX(P2D)>=12");
    ASSERT_STATEMENT_EXECUTE (DbResult::BE_SQLITE_ROW);
    ASSERT_DOUBLE (0, 12.5);

    STATEMENT_PREPARE_SUCCESS ("SELECT GetX(P3D),GetY(P3D),GetZ(P3D) FROM ecsql.P WHERE GetX(P3D)>=2 AND GetY(P3D)<=5");
    ASSERT_STATEMENT_EXECUTE (DbResult::BE_SQLITE_ROW);
    ASSERT_DOUBLE (0, 3);
    ASSERT_DOUBLE (1, 4);
    ASSERT_DOUBLE (2, 5);
    }



END_ECDBUNITTESTS_NAMESPACE