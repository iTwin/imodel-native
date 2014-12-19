/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECSqlStatement_Tests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> CreateECInstance_S4 (ECDbR ecdb, int n)
    {
    ECClassP s4;
    auto getECClassStatus = ecdb.GetEC ().GetSchemaManager ().GetECClass (s4, "NestedStructArrayTest", "S4");
    BeAssert (getECClassStatus == BentleyStatus::SUCCESS);
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
    ECClassP s3;
    auto getECClassStatus = ecdb.GetEC ().GetSchemaManager ().GetECClass (s3, "NestedStructArrayTest", "S3");
    BeAssert (getECClassStatus == BentleyStatus::SUCCESS);
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
    ECClassP s2;
    auto getECClassStatus = ecdb.GetEC ().GetSchemaManager ().GetECClass (s2, "NestedStructArrayTest", "S2");
    BeAssert (getECClassStatus == BentleyStatus::SUCCESS);
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
    ECClassP s1;
    auto getECClassStatus = ecdb.GetEC ().GetSchemaManager ().GetECClass (s1, "NestedStructArrayTest", "S1");
    BeAssert (getECClassStatus == BentleyStatus::SUCCESS);
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
    ECClassP classP;
    auto getECClassStatus = ecdb.GetEC ().GetSchemaManager ().GetECClass (classP, "NestedStructArrayTest", "ClassP");
    BeAssert (getECClassStatus == BentleyStatus::SUCCESS);
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
    auto& ecdb = SetUp ("NestedStructArrayTest.ecdb", L"NestedStructArrayTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    
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
    auto& ecdb = SetUp ("NestedStructArrayTest.ecdb", L"NestedStructArrayTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

    auto in = CreateECInstance_ClassP (ecdb, 1);

    for (auto inst : in)
        {
        ECInstanceInserter inserter (ecdb, inst->GetClass ());
        auto st = inserter.Insert (*inst);
        ASSERT_TRUE (st == BentleyStatus::SUCCESS);
        }


    ECClassP classP;
    auto getECClassStatus = ecdb.GetEC ().GetSchemaManager ().GetECClass (classP, "NestedStructArrayTest", "ClassP");
    BeAssert (getECClassStatus == BentleyStatus::SUCCESS);
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
    auto& ecdb = SetUp ("test.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), 0);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(?, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    
    statement.BindInt64 (1, 1000);
    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray (2, (UInt32) count);
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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "UPDATE  ONLY ecsql.PSA SET L = ?,  PStruct_Array = ? WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    statement.BindInt (3, 123);
    statement.BindInt64 (1, 1000);
    //add three array elements
    const UInt32 arraySize = 3;
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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    statement.BindInt (1, 123);

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_BindPrimitiveArray)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
        
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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

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

        EventHandler eh;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) statement.RegisterEventHandler (eh));
        auto stepStat = statement.Step ();

        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStat);
        ASSERT_EQ (1, eh.GetRowsAffected ());

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
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_GetParameterIndex)
    {
    const auto perClassRowCount = 10;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes), perClassRowCount);

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
    // @bsiclass                                    Muhammad.zaighum                 08/14
    //+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayInsert)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
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
    auto& arrayBinder = saStructBinder.GetMember(L"PStruct_Array").BindArray((UInt32)count);
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

//---------------------------------------------------------------------------------------
// @bsiclass                                    Muhammad.zaighum                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_StructArrayInsertWithParametersLongAndArray)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(123, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

   //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray(1, (UInt32)count);
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
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
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
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
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


TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayInsertWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.BindArray(1,(UInt32)count);
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





TEST_F(ECSqlTestFixture, ECSqlStatement_StructUpdateWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
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
TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayUpdateWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement insertStatement;
    auto stat = insertStatement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << insertStatement.GetLastStatusMessage();

    //add three array elements
    int count = 3;
    auto& arrayBinder = insertStatement.BindArray(1, (UInt32)count);
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
    auto& updateArrayBinder = updateStatement.BindArray(1, (UInt32)count);
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



/*
TEST_F(ECSqlTestFixture, ECSqlStatement_ClassWithStructHavingSructArrayUpdateWithDotoperator)
    {
    const auto perClassRowCount = 0;
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage();

    //add three array elements
    int count = 3;
    auto& arrayBinder = statement.BindArray(1, (UInt32)count);
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
    auto prepareStatus = statement.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        auto &pStructArray = statement.GetValue(0).GetArray();
        ASSERT_EQ(count, pStructArray.GetArrayLength());

        }
    statement.Finalize();
    ecsql = "UPDATE  ONLY ecsql.SA SET SAStructProp.PStruct_Array = ? ";
    prepareStatus = statement.Prepare(ecdb, ecsql);
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);

    count = 3;
    arrayBinder = statement.BindArray(1, (UInt32)count);
    ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "BindArray failed: " << statement.GetLastStatusMessage();
    for (int i = 0; i < count; i++)
        {
        auto& structBinder = arrayBinder.AddArrayElement().BindStruct();
        ASSERT_EQ((int)ECSqlStatus::Success, (int)statement.GetLastStatus()) << "AddArrayElement failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"d").BindDouble(-count);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"i").BindInt(-count);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"l").BindInt64(-count);
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        //stat = structBinder.GetMember (L"l").BindText ("Hello", IECSqlBinder::MakeCopy::No);
        //ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage ();
        stat = structBinder.GetMember(L"p2d").BindPoint2D(DPoint2d::From(i, i + 1));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        stat = structBinder.GetMember(L"p3d").BindPoint3D(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ((int)ECSqlStatus::Success, (int)stat) << "Bind to struct member failed: " << statement.GetLastStatusMessage();
        }

    stepStatus = statement.Step();
    ASSERT_EQ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage();
    statement.Finalize();

    prepareStatus = statement.Prepare(ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        auto &pStructArray = statement.GetValue(0).GetArray();
        ASSERT_EQ(count, pStructArray.GetArrayLength());

        }
    statement.Finalize();

    }*/
END_ECDBUNITTESTS_NAMESPACE
