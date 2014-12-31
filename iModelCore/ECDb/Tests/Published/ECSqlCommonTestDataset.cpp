/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlCommonTestDataset.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlCommonTestDataset.h"
#include "ECSqlStatementCrudTestDatasetHelper.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto abstractClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "Abstract");
    Utf8String ecsqlStub;
    if (ToECSql (ecsqlStub, ecsqlType, *abstractClass, true)) //polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE I > 0", ecsqlStub.c_str ());
        //Abstract class has 1 subclass, so row count per class expected
        if (ecsqlType == ECSqlType::Select)
            AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);
        else
            ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported);


        ecsql.Sprintf ("%s WHERE ECInstanceId < 0", ecsqlStub.c_str ());
        if (ecsqlType == ECSqlType::Select)
            AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0); //where cond always false
        else
            ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported);
        }

    if (ToECSql (ecsqlStub, ecsqlType, *abstractClass, false)) // non-polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE I > 0", ecsqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsqlStub.c_str (), 0); //non-polymorphic ECSQL on abstract class always returns 0

        ecsql.Sprintf ("%s WHERE ECInstanceId < 0", ecsqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    auto abstractNoSubclassesClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "AbstractNoSubclasses");
    if (ToECSql (ecsqlStub, ecsqlType, *abstractNoSubclassesClass, true)) // polymorphic
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE I > 0", ecsqlStub.c_str ());
        if (ecsqlType == ECSqlType::Select)
            AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0); //where cond always false
        else
            ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf ("%s WHERE ECInstanceId < 0", ecsqlStub.c_str ());
        if (ecsqlType == ECSqlType::Select)
            AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0); //no subclasses -> no rows
        else
            ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported);
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
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto pClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "P");
    Utf8String pClassECSqlStub;
    if (ToECSql (pClassECSqlStub, ecsqlType, *pClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE L < 3.14", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE (L < 3.14 AND I > 3) OR B = True AND D > 0.0", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        //unary operator
        ecsql.Sprintf ("%s WHERE -I = -123", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE I == 10", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::Invalid, "The only equality operator supported in SQL is the single =.");

        ecsql.Sprintf ("%s WHERE Garbage = 'bla'", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::Invalid,
            "One of the properties does not exist in the target class.");


        //*******************************************************
        //  Unsupported literals
        //*******************************************************
        ecsql.Sprintf ("%s WHERE B = UNKNOWN", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (),
            IECSqlExpectedResult::Category::Invalid, "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql.Sprintf ("%s WHERE Dt = TIME '13:35:16'", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (),
            IECSqlExpectedResult::Category::Invalid, "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.");

        ecsql.Sprintf ("%s WHERE Dt = LOCALTIME", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (),
            IECSqlExpectedResult::Category::Invalid, "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.");

        ecsql.Sprintf ("%s WHERE P2D = POINT2D (-1.3, 45.134)", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported);

        ecsql.Sprintf ("%s WHERE P3D = POINT3D (-1.3, 45.134, 2)", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported);
        }

    return std::move (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto testClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "PASpatial");
    Utf8String testClassECSqlStub;
    if (ToECSql (testClassECSqlStub, ecsqlType, *testClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE Geometry IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE Geometry IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE Geometry_Array IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE Geometry_Array IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    testClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "SSpatial");
    if (ToECSql (testClassECSqlStub, ecsqlType, *testClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE PASpatialProp.I = 123 AND PASpatialProp.Geometry IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE PASpatialProp.I = 123 AND PASpatialProp.Geometry IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE PASpatialProp.I = 123 AND PASpatialProp.Geometry_Array IS NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE PASpatialProp.I = 123 AND PASpatialProp.Geometry_Array IS NOT NULL", testClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return std::move (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlTestDataset ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto pClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "P");
    const auto pClassId = pClass->GetId ();

    Utf8String pClassECSqlStub;
    if (ToECSql (pClassECSqlStub, ecsqlType, *pClass, false))
        {
        Utf8String ecsql;
        ecsql.Sprintf ("%s WHERE GetECClassId () <> %lld", pClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE GetECClassId () = %lld", pClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE LOWER (S) = UPPER (S)", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE LOWER (UPPER (S)) = LOWER (S)", pClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE LOWER (I) = 'hello'", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::Invalid);

        ecsql.Sprintf ("%s WHERE UPPER (D) = 'hello'", pClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::Invalid);

        {
        ecsql.Sprintf ("%s WHERE LOWER (S) = ?", pClassECSqlStub.c_str ());
        auto& testItem = AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue ("sample string")));
        }

        {
        ecsql.Sprintf ("%s WHERE LOWER (S) = ?", pClassECSqlStub.c_str ());
        auto& testItem = AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (1)));
        }

        {
        ecsql.Sprintf ("%s WHERE LOWER (S) = ?", pClassECSqlStub.c_str ());
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DateTime (2012, 1, 1))));
        }

        {
        ecsql.Sprintf ("%s WHERE UPPER (?) = 'hello'", pClassECSqlStub.c_str ());
        auto& testItem = ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::Invalid);
        testItem.AddParameterValue (ECSqlTestItem::ParameterValue (ECValue (DateTime (2012,1,1))));
        }

        }

    return dataset;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "PSA");
    auto pClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "P");

    ECInstanceId psaECInstanceId;
    ECInstanceId pECInstanceId;
    ECInstanceId psaHasPECInstanceId;

    {
    auto& ecdb = testProject.GetECDb ();
    Savepoint savepoint (ecdb, "Inserting test instances", true);
    psaECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    pECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'Test instance for relationship tests')");

    Utf8String ecsql;
    ecsql.Sprintf ("INSERT INTO ecsql.PSAHasP (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld)", psaECInstanceId.GetValue(), pECInstanceId.GetValue());
    psaHasPECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, ecsql.c_str ());
    if (!psaECInstanceId.IsValid () || !pECInstanceId.IsValid () || !psaHasPECInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }

    auto psaHasPClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "PSAHasP");
    Utf8String psaHasPClassECSqlStub;
    if (ToECSql (psaHasPClassECSqlStub, ecsqlType, *psaHasPClass, false))
        {
        Utf8String ecsql;

        //using empty where clause
        AddTestItem (dataset, ecsqlType, psaHasPClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %lld", psaHasPClassECSqlStub.c_str (), psaHasPECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId <> %lld", psaHasPClassECSqlStub.c_str (), psaHasPECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld", psaHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %lld", psaHasPClassECSqlStub.c_str (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld AND TargetECInstanceId = %lld", psaHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld AND TargetECInstanceId <> %lld", psaHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld AND TargetECClassId = %lld", psaHasPClassECSqlStub.c_str (), psaClassId, pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld + 1 AND TargetECClassId = %lld", psaHasPClassECSqlStub.c_str (), psaClassId, pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "PSA");
    
    ECInstanceId psaECInstanceId1;
    ECInstanceId psaECInstanceId2;
    ECInstanceId psaHasPsaECInstanceId;
    {
    auto& ecdb = testProject.GetECDb ();
    Savepoint savepoint (ecdb, "Inserting test instances", true);
    psaECInstanceId1 = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (100, 'Test instance for relationship tests')");
    psaECInstanceId2 = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (200, 'Second test instance for relationship tests')");

    Utf8String ecsql;
    ecsql.Sprintf ("INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES (%lld, %lld)", psaECInstanceId1.GetValue(), psaECInstanceId2.GetValue());
    psaHasPsaECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, ecsql.c_str ());
    if (!psaECInstanceId1.IsValid () || !psaECInstanceId2.IsValid () || !psaHasPsaECInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }


    auto psaHasPsaClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "PSAHasPSA");
    Utf8String psaHasPsaClassECSqlStub;
    if (ToECSql (psaHasPsaClassECSqlStub, ecsqlType, *psaHasPsaClass, false))
        {
        Utf8String ecsql;

        //using empty where clause
        AddTestItem (dataset, ecsqlType, psaHasPsaClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %lld", psaHasPsaClassECSqlStub.c_str (), psaHasPsaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId <> %lld", psaHasPsaClassECSqlStub.c_str (), psaHasPsaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId1.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %lld", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId2.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld AND TargetECInstanceId = %lld", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId1.GetValue (), psaECInstanceId2.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld AND TargetECInstanceId <> %lld", psaHasPsaClassECSqlStub.c_str (), psaECInstanceId1.GetValue (), psaECInstanceId2.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld AND TargetECClassId = %lld", psaHasPsaClassECSqlStub.c_str (), psaClassId, psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld + 1 AND TargetECClassId = %lld", psaHasPsaClassECSqlStub.c_str (), psaClassId, psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "PSA");
    auto pClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "P");

    ECInstanceId ecInstanceId;
    {
    auto& ecdb = testProject.GetECDb ();
    Savepoint savepoint (ecdb, "Inserting test instances", true);
    ecInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId, B, D) VALUES (100, 200, False, 3.14)");
    if (!ecInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }

    auto relClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "PSAHasPWithPrimProps");
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

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = 100", relClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = 200", relClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECClassId <> %lld", relClassECSqlStub.c_str (), psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld", relClassECSqlStub.c_str (), psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECClassId <> %lld", relClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE TargetECClassId = %lld", relClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %lld", relClassECSqlStub.c_str (), ecInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId > %lld", relClassECSqlStub.c_str (), ecInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "PSA");
    auto pClassId = ECSqlStatementCrudTestDatasetHelper::GetClassId (testProject.GetTestSchemaManager (), "ECSqlTest", "P");

    ECInstanceId pECInstanceId;
    ECInstanceId psaECInstanceId (1000LL); //can be fictitious as it is not checked by ECDb
    ECInstanceId psaHasAnyClassECInstanceId;
    ECInstanceId anyClassHasPECInstanceId;
    {
    auto& ecdb = testProject.GetECDb ();
    Savepoint savepoint (ecdb, "Inserting test instances", true);
    pECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (100, 'test instance')");

    Utf8String ecsql;
    ecsql.Sprintf ("INSERT INTO ecsql.PSAHasAnyClass_0N (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (%lld, 200, %lld)", psaECInstanceId.GetValue(), pClassId);
    psaHasAnyClassECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, ecsql.c_str ());

    //this is an end-table mapping. Therefore the source ecinstanceid must match the end table's row
    ecsql.Sprintf ("INSERT INTO ecsql.AnyClassHasP_0N (SourceECInstanceId, SourceECClassId, TargetECInstanceId) VALUES (%lld, %lld, %lld)", psaECInstanceId.GetValue(), psaClassId, pECInstanceId.GetValue());
    anyClassHasPECInstanceId = ECSqlStatementCrudTestDatasetHelper::InsertTestInstance (ecdb, ecsql.c_str ());

    if (!pECInstanceId.IsValid () || !psaHasAnyClassECInstanceId.IsValid () || !anyClassHasPECInstanceId.IsValid ())
        {
        savepoint.Cancel ();
        return dataset;
        }

    savepoint.Commit ();
    }

    //******* AnyClass on Target end ***********
    auto psaHasAnyClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "PSAHasAnyClass_0N");
    Utf8String psaHasAnyClassECSqlStub;
    if (ToECSql (psaHasAnyClassECSqlStub, ecsqlType, *psaHasAnyClass, false))
        {
        Utf8String ecsql;

        //no where clause
        AddTestItem (dataset, ecsqlType, psaHasAnyClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld", psaHasAnyClassECSqlStub.c_str (), psaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld + 1", psaHasAnyClassECSqlStub.c_str (), psaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = 200", psaHasAnyClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = 201", psaHasAnyClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %lld", psaHasAnyClassECSqlStub.c_str (), psaHasAnyClassECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId <> %lld", psaHasAnyClassECSqlStub.c_str (), psaHasAnyClassECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld", psaHasAnyClassECSqlStub.c_str (), psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        //source ecclass is PSA, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld", psaHasAnyClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        //source ecclass is PSA, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE SourceECClassId = 99999999999", psaHasAnyClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE TargetECClassId = %lld", psaHasAnyClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        //target ecclass for the test instance is P, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE TargetECClassId = %lld", psaHasAnyClassECSqlStub.c_str (), psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        //target ecclass for the test instance is P, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE TargetECClassId = 99999999999", psaHasAnyClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    //******* AnyClass on Source end ***********
    auto anyHasPClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "AnyClassHasP_0N");
    Utf8String anyHasPClassECSqlStub;
    if (ToECSql (anyHasPClassECSqlStub, ecsqlType, *anyHasPClass, false))
        {
        Utf8String ecsql;

        //empty where clause
        AddTestItem (dataset, ecsqlType, anyHasPClassECSqlStub.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId = %lld", anyHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE SourceECInstanceId <> %lld", anyHasPClassECSqlStub.c_str (), psaECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %lld", anyHasPClassECSqlStub.c_str (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE TargetECInstanceId = %lld + 1", anyHasPClassECSqlStub.c_str (), pECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE ECInstanceId = %lld", anyHasPClassECSqlStub.c_str (), anyClassHasPECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        ecsql.Sprintf ("%s WHERE ECInstanceId <> %lld", anyHasPClassECSqlStub.c_str (), anyClassHasPECInstanceId.GetValue ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld", anyHasPClassECSqlStub.c_str (), psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        //source ecclass for the test instance is PSA, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE SourceECClassId = %lld", anyHasPClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        //source ecclass for the test instance is PSA, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE SourceECClassId = 99999999999", anyHasPClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        ecsql.Sprintf ("%s WHERE TargetECClassId = %lld", anyHasPClassECSqlStub.c_str (), pClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 1);

        //target ecclass is P, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE TargetECClassId = %lld", anyHasPClassECSqlStub.c_str (), psaClassId);
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);

        //target ecclass is P, so any other class id should result in 0 deletes
        ecsql.Sprintf ("%s WHERE TargetECClassId = 99999999999", anyHasPClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), 0);
        }

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlCommonTestDataset::WhereStructTests (ECSqlType ecsqlType, ECDbTestProject& testProject, int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    auto psaClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "PSA");
    Utf8String psaClassECSqlStub;
    if (ToECSql (psaClassECSqlStub, ecsqlType, *psaClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE PStructProp IS NULL", psaClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported, "Structs are not supported yet in where clause.");

        ecsql.Sprintf ("%s WHERE PStructProp IS NOT NULL", psaClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported, "Structs are not supported yet in where clause.");

        ecsql.Sprintf ("%s WHERE PStructProp = ?", psaClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported, "Structs are not supported yet in where clause.");

        ecsql.Sprintf ("%s WHERE PStructProp.i = 123 AND B = true", psaClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);

        ecsql.Sprintf ("%s WHERE PStructProp.i = 123 AND PStructProp.dt <> DATE '2010-10-10' AND B = true", psaClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);
        }

    auto saClass = testProject.GetTestSchemaManager ().GetClass ("ECSqlTest", "SA");
    Utf8String saClassECSqlStub;
    if (ToECSql (saClassECSqlStub, ecsqlType, *saClass, false))
        {
        Utf8String ecsql;

        ecsql.Sprintf ("%s WHERE SAStructProp.PStructProp IS NULL", saClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported, "Structs are not supported yet in where clause.");

        ecsql.Sprintf ("%s WHERE SAStructProp.PStructProp = ?", saClassECSqlStub.c_str ());
        ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql.c_str (), IECSqlExpectedResult::Category::NotYetSupported, "Structs are not supported yet in where clause.");

        ecsql.Sprintf ("%s WHERE SAStructProp.PStructProp.i = 123 AND SAStructProp.PStructProp.dt <> DATE '2010-10-10'", saClassECSqlStub.c_str ());
        AddTestItem (dataset, ecsqlType, ecsql.c_str (), rowCountPerClass);
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
        return ECSqlStatementCrudTestDatasetHelper::AddSelect (dataset, ecsql, 1, expectedResultRows);
    else
        return ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, expectedResultRows, true);
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
                ECSqlSelectBuilder ecsqlBuilder;
                ecsqlBuilder.Select (ECSqlSelectBuilder::ECINSTANCEID_SYSTEMPROPERTY).From (targetClass, polymorphic);
                ecsql = ecsqlBuilder.ToString ();
                return true;
                }

            case ECSqlType::Insert:
                {
                ECSqlInsertBuilder ecsqlBuilder;
                ecsqlBuilder.InsertInto (targetClass);
                ecsql = ecsqlBuilder.ToString ();
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

                ECSqlUpdateBuilder ecsqlBuilder;
                ecsqlBuilder.Update (targetClass, polymorphic).AddSet (propAccessString.c_str (), "NULL");
                ecsql = ecsqlBuilder.ToString ();
                return true;
                }

            case ECSqlType::Delete:
                {
                ECSqlDeleteBuilder ecsqlBuilder;
                ecsqlBuilder.DeleteFrom (targetClass, polymorphic);
                ecsql = ecsqlBuilder.ToString ();
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
    for (auto prop : ecClass.GetProperties (includeBaseProperties))
        {
        //ECSQL doesn't handle arrays yet, so always skip those when trying to find a test property access string
        if (prop->GetIsArray ())
            continue;

        //if this is the first recursion level, no . separator needed
        if (!propertyAccessString.empty ())
            propertyAccessString.append (".");

        propertyAccessString.append (Utf8String (prop->GetName ()));

        if (prop->GetIsPrimitive ())
            return true;
        else if (prop->GetIsStruct ())
            {
            auto structProp = prop->GetAsStructProperty ();
            return FindPrimitivePropertyAccessStringInClass (propertyAccessString, structProp->GetType (), includeBaseProperties);
            }
        }

    return false;
    }


END_ECDBUNITTESTS_NAMESPACE