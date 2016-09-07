/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCommonTestDataset.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlCommonTestDataset.h"
#include "ECSqlTestFrameworkHelper.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::CasingTests(ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8String ecsql;
    switch (ecsqlType)
        {
            case ECSqlType::Delete:
            {
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY EcSqltEst.P", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ECSQLTEST.P", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ecsqltest.P", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ECSqlTest.p", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ECSqlTest.P WHERE i<0", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ecsqltest.p WHERE i<0", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY Ecsql.P", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ecSql.P", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ecsql.p", 0);
            AddTestItem(dataset, ecsqlType, "DELETE FROM ONLY ecsql.P WHERE i<0", 0);
            break;
            }
            case ECSqlType::Insert:
            {
            AddTestItem(dataset, ecsqlType, "INSERT INTO EcSqltEst.P(ECInstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ECSQLTEST.P(ECInstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ecsqltest.P(ECInstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ECSqlTest.p(ECInstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ecsQl.P(ECInstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ecsQl.p(ECInstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ecsQl.p(EcinstanceId) VALUES(NULL)", 0);
            AddTestItem(dataset, ecsqlType, "INSERT INTO ecsQl.p(ecinstanceId) VALUES(NULL)", 0);
            break;
            }

            case ECSqlType::Select:
            {
            AddTestItem(dataset, ecsqlType, "SELECT S FROM EcSqltEst.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT S FROM ECSQLTEST.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT S FROM ecsqltest.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT S FROM ECSqlTest.p", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT s FROM ECSqlTest.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT S FROM ecsQl.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT S FROM ecsql.p", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT s FROM ecsql.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT ecinsTanceiD FROM ECSqlTest.P", rowCountPerClass);
            AddTestItem(dataset, ecsqlType, "SELECT getecCLassid() FROM ECSqlTest.P", rowCountPerClass);
            break;
            }
            case ECSqlType::Update:
            {
            AddTestItem(dataset, ecsqlType, "UPDATE ECSqlTest.P SET I=?", 0);
            AddTestItem(dataset, ecsqlType, "UPDATE EcSqltEst.P SET I=?", 0);
            AddTestItem(dataset, ecsqlType, "UPDATE ECSQLTEST.P SET I=?", 0);
            AddTestItem(dataset, ecsqlType, "UPDATE ECSqlTest.p SET I=?", 0);
            AddTestItem(dataset, ecsqlType, "UPDATE ecsql.p SET I=?", 0);
            AddTestItem(dataset, ecsqlType, "UPDATE ecsQl.P SET I=?", 0);
            AddTestItem(dataset, ecsqlType, "UPDATE ecsql.P SET i=?", 0);
            break;
            }

            default:
                BeAssert(false);
        }

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto abstractClass = ecdb.Schemas().GetECClass("ECSqlTest", "AbstractTablePerHierarchy");
    Utf8String ecsqlStub;
    if (ToECSql(ecsqlStub, ecsqlType, *abstractClass, true)) //polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf("%s WHERE I > 0", ecsqlStub.c_str());
        if (ecsqlType == ECSqlType::Delete)
            AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        else
            {
            //AbstractTablePerHierarchy class has 2 subclasses, so row count per class expected
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 2 * rowCountPerClass);
            }

        ecsql.Sprintf("%s WHERE ECInstanceId < 0", ecsqlStub.c_str());
        if (ecsqlType == ECSqlType::Delete)
            AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        else
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0); //where cond always false
        }

    if (ToECSql (ecsqlStub, ecsqlType, *abstractClass, false)) // non-polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE I > 0", ecsqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsqlStub.c_str (), 0); //non-polymorphic ECSQL on abstract class always returns 0

        ecsql.Sprintf ("%s WHERE ECInstanceId < 0", ecsqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    auto abstractNoSubclassesClass = ecdb.Schemas().GetECClass("ECSqlTest", "AbstractNoSubclasses");
    if (ToECSql(ecsqlStub, ecsqlType, *abstractNoSubclassesClass, true)) // polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf("%s WHERE I > 0", ecsqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0); //where cond always false

        ecsql.Sprintf("%s WHERE ECInstanceId < 0", ecsqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0); //no subclasses -> no rows
        }

    if (ToECSql (ecsqlStub, ecsqlType, *abstractNoSubclassesClass, false)) // non-polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE I > 0", ecsqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsqlStub.c_str (), 0); //no subclasses -> no rows

        ecsql.Sprintf ("%s WHERE ECInstanceId < 0", ecsqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0); //no subclasses -> no rows
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "TH3");
    Utf8String testClassECSqlStub;
    if (ToECSql(testClassECSqlStub, ecsqlType, *testClass, false))
        {
        Utf8String ecsql;
        ecsql.Sprintf("%s WHERE S1 IS NOT NULL OR S2 IS NOT NULL", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE (S1 IS NOT NULL OR S2 IS NOT NULL)", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE S1 IS NULL AND S2 IS NOT NULL", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE S1 IS NULL AND S2 IS NOT NULL OR 1=1", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE S1 IS NULL AND (S2 IS NOT NULL OR 1=1)", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto pClass = ecdb.Schemas().GetECClass("ECSqlTest", "P");
    Utf8String pClassECSqlStub;
    if (ToECSql (pClassECSqlStub, ecsqlType, *pClass, false))
        {
        Utf8String ecsql;

        //case insensitive tests
        ecsql.Sprintf("%s WHERE B = NULL OR b = NULL", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        {
        ecsql.Sprintf("%s WHERE i>=:myParam", pClassECSqlStub.c_str());
        auto& testItem = AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);
        testItem.AddParameterValue(ECSqlTestItem::ParameterValue("MypaRaM", ECValue(1)));
        }

        ecsql.Sprintf("%s WHERE I IS 123", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE B IS TRUE", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE B = NULL", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE B <> NULL", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf ("%s WHERE L < 3.14", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE (L < 3.14 AND I > 3) OR B = True AND D > 0.0", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf("%s WHERE 8 %% 3 = 2", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE 8 %% 2 = 0", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE (I&1)=1 AND ~(I|2=I)", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE 5 + (4&1) = 5", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE 5 + 4 & 1 = 1", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE 5 + 4 | 1 = 9", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE 4|1&1 = 5", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE (4|1)&1 = 1", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE 4^1 = 0", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf("%s WHERE 5^4 = 4", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::NotYetSupported);

        //unary predicates
        ecsql.Sprintf("%s WHERE True", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE NOT True", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE B", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE NOT B", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        //SQLite function which ECDb knows to return a bool
        ecsql.Sprintf("%s WHERE Glob('*amp*',S)", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE NOT Glob('*amp*',S)", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        //Int/Long types are supported as unary predicate. They evalute to True if they are not 0.
        ecsql.Sprintf("%s WHERE I", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE NOT I", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE L", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE NOT L", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE Length(S)", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE NOT Length(S)", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE (I IS NOT NULL) AND L", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE (I IS NOT NULL) AND NOT L", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE 3.14", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE 'hello'", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE D", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE S", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE P2D", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE P3D", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE ?", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE Random()", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE Hex(Bi)", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        //unary operator
        ecsql.Sprintf ("%s WHERE -I = -123", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE I == 10", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (), ECSqlExpectedResult::Category::Invalid, "The only equality operator supported in SQL is the single =.");

        ecsql.Sprintf ("%s WHERE Garbage = 'bla'", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (), ECSqlExpectedResult::Category::Invalid,
            "One of the properties does not exist in the target class.");


        //*******************************************************
        //  Unsupported literals
        //*******************************************************
        ecsql.Sprintf ("%s WHERE B = UNKNOWN", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (),
            ECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql.Sprintf ("%s WHERE Dt = TIME '13:35:16'", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (),
            ECSqlExpectedResult::Category::Invalid, "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql.Sprintf ("%s WHERE Dt = LOCALTIME", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (),
            ECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

        ecsql.Sprintf ("%s WHERE P2D = POINT2D (-1.3, 45.134)", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (), ECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf ("%s WHERE P3D = POINT3D (-1.3, 45.134, 2)", pClassECSqlStub.c_str ());
        ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (), ECSqlExpectedResult::Category::NotYetSupported);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto testClass = ecdb.Schemas().GetECClass("ECSqlTest", "PASpatial");
    Utf8String testClassECSqlStub;
    if (ToECSql (testClassECSqlStub, ecsqlType, *testClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf("%s WHERE Geometry=?", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE Geometry<>?", testClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0); //NULL<>NULL is always false

        ecsql.Sprintf ("%s WHERE Geometry IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE Geometry IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE Geometry_Array IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE Geometry_Array IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    testClass = ecdb.Schemas().GetECClass("ECSqlTest", "SSpatial");
    if (ToECSql (testClassECSqlStub, ecsqlType, *testClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE SpatialStructProp.Geometry IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SpatialStructProp.Geometry IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE SpatialStructProp.Geometry_Array IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE SpatialStructProp.Geometry_Array IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    ECClassCP pClass = ecdb.Schemas().GetECClass("ECSqlTest", "P");
    const ECClassId pClassId = pClass->GetId ();

    Utf8String pClassECSqlStub;
    if (ToECSql (pClassECSqlStub, ecsqlType, *pClass, false))
        {
        Utf8String ecsql;
        ecsql.Sprintf ("%s WHERE GetECClassId() <> %s", pClassECSqlStub.c_str (), pClassId.ToString().c_str());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf("%s WHERE ECClassId <> %s", pClassECSqlStub.c_str(), pClassId.ToString().c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf ("%s WHERE GetECClassId() = %s", pClassECSqlStub.c_str (), pClassId.ToString().c_str());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf("%s WHERE ECClassId = %s", pClassECSqlStub.c_str(), pClassId.ToString().c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE LOWER(S) = UPPER(S)", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE LOWER(UPPER(S)) = LOWER (S)", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        //lower/upper only make sense with strings, but no failure if used for other data types (like in SQLite)
        ecsql.Sprintf ("%s WHERE LOWER(I)=I", pClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE UPPER(D)>0", pClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        {
        ecsql.Sprintf ("%s WHERE LOWER(S)=?", pClassECSqlStub.c_str ());
        auto& testItem = AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("sample string")));
        }

        {
        ecsql.Sprintf ("%s WHERE LOWER(S)=?", pClassECSqlStub.c_str ());
        auto& testItem = AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql.Sprintf ("%s WHERE LOWER(S)=?", pClassECSqlStub.c_str ());
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (), ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DateTime (2012, 1, 1))));
        }

        {
        ecsql.Sprintf ("%s WHERE UPPER(?) = 'hello'", pClassECSqlStub.c_str ());
        auto& testItem = ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql.c_str (), ECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DateTime (2012,1,1))));
        }

        ecsql.Sprintf("%s WHERE GetX(P2D) >= -11.111", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE GetY(P2D) >= -11.111", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE GetZ(P2D) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);
        
        ecsql.Sprintf("%s WHERE GetX(P3D) >= -11.111", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE GetY(P3D) >= -11.111", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE GetZ(P3D) >= -11.111", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE GetX(P2D) >= GetX(P3D) AND GetY(P2D) >= GetY(P3D)", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        //with parentheses around function calls
        ecsql.Sprintf("%s WHERE (GetX(P2D)) >= (GetX(P3D)) AND (GetY(P2D)) >= (GetY(P3D))", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE (GetX(P2D) >= GetX(P3D)) AND (GetY(P2D) >= GetY(P3D))", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE (GetX(P2D) >= GetX(P3D) AND GetY(P2D) >= GetY(P3D))", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf("%s WHERE GetX(?) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE GetY(?) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE GetZ(?) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE GetX(NULL) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE GetX(Bi) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE GetX(D) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE GetX(S) >= -11.111", pClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE InVirtualSet(?, ECInstanceId)", pClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);
        }

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereMatchTests(ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "P");
    Utf8String testClassECSqlStub;
    if (ToECSql(testClassECSqlStub, ecsqlType, *testClass, false))
        {
        //This uses a dummy function to just test that parsing and preparing works. In real usage the function
        //on the rhs must be a geometric function as defined in SQLite
        Utf8String ecsql;
        ecsql.Sprintf("%s WHERE ECInstanceId MATCH random()", testClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE ECInstanceId NOT MATCH random()", testClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE I MATCH random()", testClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        //even though SQLite expects the LHS to be a column, we allow a value exp in the ECSQL grammar.
        ecsql.Sprintf("%s WHERE (I + L) MATCH random()", testClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddStepFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf("%s WHERE ECInstanceId MATCH '123'", testClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid);
        }

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType ecsqlType, ECDbR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ecdb.Schemas().GetECClassId ("ECSqlTest", "PSA");
    auto pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId psaHasPECInstanceId;

    {
    Savepoint savepoint (ecdb, "Inserting test instances");
    psaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");

    Utf8String ecsql;
    ecsql.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%llu, %llu)", psaECInstanceId.GetValue(), pECInstanceId.GetValue());
    psaHasPECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, ecsql.c_str ());
    if (!psaECInstanceId.IsValid () || !pECInstanceId.IsValid () || !psaHasPECInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }

    auto psaHasPClass = ecdb.Schemas().GetECClass ("ECSqlTest", "PSAHasP");
    Utf8String psaHasPClassECSqlStub;
    if (ToECSql (psaHasPClassECSqlStub, ecsqlType, *psaHasPClass, false))
        {
        Utf8String ecsql;

        //using empty where clause
        AddTestItem (dataset, ecsqlType, psaHasPClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %llu", psaHasPClassECSqlStub.c_str (), psaHasPECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId <> %llu", psaHasPClassECSqlStub.c_str (), psaHasPECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu", psaHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %llu", psaHasPClassECSqlStub.c_str (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu AND TargetECInstanceId = %llu", psaHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu AND TargetECInstanceId <> %llu", psaHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %llu AND TargetECClassId = %llu", psaHasPClassECSqlStub.c_str (), psaClassId.GetValue(), pClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %llu + 1 AND TargetECClassId = %llu", psaHasPClassECSqlStub.c_str (), psaClassId.GetValue(), pClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType ecsqlType, ECDbR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    
    ECInstanceId psaECInstanceId1;
    ECInstanceId psaECInstanceId2;
    ECInstanceId psaHasPsaECInstanceId;
    {
    Savepoint savepoint (ecdb, "Inserting test instances");
    psaECInstanceId1 = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    psaECInstanceId2 = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (200, 'Second test instance for relationship tests')");

    Utf8String ecsql;
    ecsql.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES (%llu, %llu)", psaECInstanceId1.GetValue(), psaECInstanceId2.GetValue());
    psaHasPsaECInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, ecsql.c_str ());
    if (!psaECInstanceId1.IsValid () || !psaECInstanceId2.IsValid () || !psaHasPsaECInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }

    auto psaHasPsaClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSAHasPSA");
    Utf8String psaHasPsaClassECSqlStub;
    if (ToECSql (psaHasPsaClassECSqlStub, ecsqlType, *psaHasPsaClass, false))
        {
        Utf8String ecsql;

        //using empty where clause
        AddTestItem (dataset, ecsqlType, psaHasPsaClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %llu", psaHasPsaClassECSqlStub.c_str (), psaHasPsaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId <> %llu", psaHasPsaClassECSqlStub.c_str (), psaHasPsaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId1.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %llu", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId2.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu AND TargetECInstanceId = %llu", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId1.GetValue (), psaECInstanceId2.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu AND TargetECInstanceId <> %llu", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId1.GetValue (), psaECInstanceId2.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %llu AND TargetECClassId = %llu", psaHasPsaClassECSqlStub.c_str (), psaClassId.GetValue(), psaClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %llu + 1 AND TargetECClassId = %llu", psaHasPsaClassECSqlStub.c_str (), psaClassId.GetValue(), psaClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType ecsqlType, ECDbR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "PSA");
    auto pClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "P");

    ECInstanceId sourceECInstanceId;
    {
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.PSA LIMIT 1"));
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
    sourceECInstanceId = stmt.GetValueId<ECInstanceId>(0);
    EXPECT_NE(sourceECInstanceId.GetValue(), 0LL);
    }

    ECInstanceId targetECInstanceId;
    {
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.P LIMIT 1"));
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
    targetECInstanceId = stmt.GetValueId<ECInstanceId>(0);
    EXPECT_NE(targetECInstanceId.GetValue(), 0LL);
    }

    ECInstanceId ecInstanceId;
    {
    Savepoint savepoint (ecdb, "Inserting test instances");
    ecInstanceId = ECSqlTestFrameworkHelper::InsertTestInstance (ecdb, SqlPrintfString("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId, B, D) VALUES (%llu, %llu, False, 3.14)", sourceECInstanceId.GetValue(),targetECInstanceId.GetValue()).GetUtf8CP());
    if (!ecInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }

    auto relClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSAHasPWithPrimProps");
    Utf8String relClassECSqlStub;
    if (ToECSql (relClassECSqlStub, ecsqlType, *relClass, false))
        {
        Utf8String ecsql;

        //no where clause
        AddTestItem (dataset, ecsqlType, relClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE B = true", relClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE B = false", relClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE B = false AND D = 3.14", relClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %llu", relClassECSqlStub.c_str (), sourceECInstanceId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %llu", relClassECSqlStub.c_str (), targetECInstanceId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECClassId <> %llu", relClassECSqlStub.c_str (), psaClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %llu", relClassECSqlStub.c_str (), psaClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECClassId <> %llu", relClassECSqlStub.c_str (), pClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE TargetECClassId = %llu", relClassECSqlStub.c_str (), pClassId.GetValue());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %llu", relClassECSqlStub.c_str (), ecInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId > %llu", relClassECSqlStub.c_str (), ecInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereStructTests (ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSA");
    Utf8String psaClassECSqlStub;
    if (ToECSql (psaClassECSqlStub, ecsqlType, *psaClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE PStructProp IS NULL", psaClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf ("%s WHERE PStructProp IS NOT NULL", psaClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE PStructProp = ?", psaClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf("%s WHERE PStructProp<>?", psaClassECSqlStub.c_str());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0); //NULL<>NULL is always false

        ecsql.Sprintf ("%s WHERE PStructProp.i = 123 AND B = true", psaClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE PStructProp.i = 123 AND PStructProp.dt <> DATE '2010-10-10' AND B = true", psaClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);
        }

    auto saClass = ecdb.Schemas().GetECClass("ECSqlTest", "SA");
    Utf8String saClassECSqlStub;
    if (ToECSql (saClassECSqlStub, ecsqlType, *saClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE SAStructProp.PStructProp IS NULL", saClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf ("%s WHERE SAStructProp.PStructProp = ?", saClassECSqlStub.c_str ());
        AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

        ecsql.Sprintf ("%s WHERE SAStructProp.PStructProp.i = 123 AND SAStructProp.PStructProp.dt <> DATE '2010-10-10'", saClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf("%s WHERE SAStructProp IS NULL", saClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "Structs with struct array props are not supported in the where clause");

        ecsql.Sprintf("%s WHERE SAStructProp IS NOT NULL", saClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "Structs with struct array props are not supported in the where clause");

        ecsql.Sprintf("%s WHERE SAStructProp=?", saClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "Structs with struct array props are not supported in the where clause");

        ecsql.Sprintf("%s WHERE SAStructProp<>?", saClassECSqlStub.c_str());
        ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "Structs with struct array props are not supported in the where clause");
        }

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::OptionsTests(ECSqlType ecsqlType, ECDbCR ecdb, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    ECClassCP pClass = ecdb.Schemas().GetECClass("ECSqlTest", "P");
    Utf8String pClassECSqlStub;
    if (ToECSql(pClassECSqlStub, ecsqlType, *pClass, false))
        {
        Utf8String ecsql;

        if (ecsqlType != ECSqlType::Insert)
            {
            ecsql.Sprintf("%s ECSQLOPTIONS", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "OPTIONS clause without options");

            ecsql.Sprintf("%s ECSQLOPTIONS 123", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "An option must be a name");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt=", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "option value is missing");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt myopt", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "duplicate options not allowed");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt myOpt", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "duplicate options not allowed (even if they differ by case)");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt=1 myopt", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "duplicate options not allowed");

            ecsql.Sprintf("%s ECSQLOPTIONS myOpt=1 myopt", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "duplicate options not allowed");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

            ecsql.Sprintf("%s ECSQLOPTIONS myopt myotheropt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

            ecsql.Sprintf("%s ECSQLOPTIONS myopt=1 myotheropt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

            ecsql.Sprintf("%s ECSQLOPTIONS myopt=1 myotheropt=true", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

            ecsql.Sprintf("%s ECSQLOPTIONS myopt myotheropt=true", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

            ecsql.Sprintf("%s ECSQLOPTIONS myopt myotheropt=true onemoreopt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), rowCountPerClass);

            ecsql.Sprintf("%s WHERE ECInstanceId=? ECSQLOPTIONS myopt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

            ecsql.Sprintf("%s WHERE ECInstanceId=? ECSQLOPTIONS myopt myotheropt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

            ecsql.Sprintf("%s WHERE ECInstanceId=? ECSQLOPTIONS myopt=1 myotheropt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);
            }

        if (ecsqlType == ECSqlType::Select)
            {
            ecsql.Sprintf("%s WHERE ECInstanceId=? ORDER BY I ECSQLOPTIONS myopt=1 myotheropt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);

            ecsql.Sprintf("%s WHERE ECInstanceId=? GROUP BY I HAVING I=1 ECSQLOPTIONS myopt=1 myotheropt", pClassECSqlStub.c_str());
            AddTestItem(dataset, ecsqlType, ecsql.c_str(), 0);
            }

        if (ecsqlType == ECSqlType::Insert)
            {
            ecsql.Sprintf("%s ECSQLOPTIONS myopt", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "No options supported for INSERT");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt=1", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "No options supported for INSERT");

            ecsql.Sprintf("%s ECSQLOPTIONS myopt myotheropt", pClassECSqlStub.c_str());
            ECSqlTestFrameworkHelper::AddPrepareFailing(dataset, ecsql.c_str(), ECSqlExpectedResult::Category::Invalid, "No options supported for INSERT");
            }

        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestItem& ECSqlCommonTestDataset::AddTestItem (ECSqlTestDataset& dataset, ECSqlType ecsqlType, Utf8CP ecsql, int expectedResultRows)
    {
    if (ecsqlType == ECSqlType::Select)
        //all select statements tested by the commonalities return a single column: SELECT ECInstanceId FROM ...
        return ECSqlTestFrameworkHelper::AddSelect (dataset, ecsql, 1, expectedResultRows);
    else
        return ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECSqlCommonTestDataset::ToECSql (Utf8StringR ecsql, ECSqlType type, ECClassCR targetClass, bool polymorphic)
    {
    switch (type)
        {
            case ECSqlType::Select:
                {
                ecsql.assign("SELECT ECInstanceId FROM ");
                if (!polymorphic)
                    ecsql.append("ONLY ");

                ecsql.append(targetClass.GetECSqlName());
                return true;
                }

            case ECSqlType::Insert:
                {
                ecsql.assign("INSERT INTO ");
                ecsql.append(targetClass.GetECSqlName());
                return true;
                }

            case ECSqlType::Update:
                {
                if (targetClass.GetPropertyCount (polymorphic) == 0)
                    return false;

                //retrieve the first property of the class and set it to NULL in the test ECSQL
                Utf8String propAccessString;
                const bool found = FindPrimitivePropertyAccessStringInClass (propAccessString, targetClass, polymorphic);
                if (!found)
                    return false;

                ecsql.assign("UPDATE ");
                if (!polymorphic)
                    ecsql.append("ONLY ");

                ecsql.append(targetClass.GetECSqlName()).append(" SET ").append(propAccessString).append("=NULL");
                return true;
                }

            case ECSqlType::Delete:
                {
                ecsql.assign("DELETE FROM ");
                if (!polymorphic)
                    ecsql.append("ONLY ");

                ecsql.append(targetClass.GetECSqlName());
                return true;
                }

            default:
                {
                BeAssert (false);
                return false;
                }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECSqlCommonTestDataset::FindPrimitivePropertyAccessStringInClass (Utf8StringR propertyAccessString, ECN::ECClassCR ecClass, bool includeBaseProperties)
    {
    for (ECPropertyCP prop : ecClass.GetProperties (includeBaseProperties))
        {
        //ECSQL doesn't handle arrays yet, so always skip those when trying to find a test property access string
        if (prop->GetIsArray ())
            continue;

        //if this is the first recursion level, no . separator needed
        if (!propertyAccessString.empty ())
            propertyAccessString.append (".");

        propertyAccessString.append (prop->GetName());

        if (prop->GetIsPrimitive ())
            return true;
        else if (prop->GetIsStruct ())
            {
            StructECPropertyCP structProp = prop->GetAsStructProperty ();
            return FindPrimitivePropertyAccessStringInClass (propertyAccessString, structProp->GetType (), includeBaseProperties);
            }
        }

    return false;
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE