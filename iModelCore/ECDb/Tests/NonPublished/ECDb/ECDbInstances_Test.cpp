/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECDbInstances_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../PublicApi/NonPublished/ECDb/ECDbTestProject.h"
#include <limits>
#include <initializer_list>
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

void ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath);
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdHelper, ECInstanceIdInstanceIdConversion)
    {
    //ToString
    ECInstanceId ecInstanceId (123456789LL);
    Utf8CP expectedInstanceId = "123456789";
    Utf8Char actualInstanceId[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    bool success = ECInstanceIdHelper::ToString (actualInstanceId, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ecInstanceId);
    EXPECT_TRUE (success);
    EXPECT_STREQ (expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValue ();

    ecInstanceId = ECInstanceId (0LL);
    expectedInstanceId = "0";
    actualInstanceId[0] = '\0';
    success = ECInstanceIdHelper::ToString (actualInstanceId, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ecInstanceId);
    EXPECT_FALSE (success);

    //ECInstanceId are allowed to be negative
    ecInstanceId = ECInstanceId (-1LL);
    success = ECInstanceIdHelper::ToString (actualInstanceId, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ecInstanceId);
    EXPECT_TRUE (success) << "ECInstanceIds can be negative, so ToString should succeed";

    ecInstanceId = ECInstanceId(-100LL);
    success = ECInstanceIdHelper::ToString(actualInstanceId, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ecInstanceId);
    EXPECT_TRUE(success) << "ECInstanceIds can be negative, so ToString should succeed";

    ecInstanceId = ECInstanceId (123LL);
    Utf8Char smallIdBuffer[10];
    ASSERT_FALSE (ECInstanceIdHelper::ToString (smallIdBuffer, (size_t) 10, ecInstanceId)) << "Expected to fail if buffer is to small to take max uint64_t as string";

    Utf8Char stillTooSmallIdBuffer[std::numeric_limits<int64_t>::digits10 + 1];
    ASSERT_FALSE (ECInstanceIdHelper::ToString (stillTooSmallIdBuffer, std::numeric_limits<int64_t>::digits10 + 1, ecInstanceId)) << "Expected to fail if buffer is to small to take max int64_t + 2 as string. Sign character and trailing 0 must be considered in the buffer size.";

    //FromString
    Utf8CP instanceId = "123456789";
    ECInstanceId expectedECInstanceId (123456789LL);
    ECInstanceId actualECInstanceId;
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_TRUE (success) << "Unexpected failure of ECInstanceIdHelper::FromString";
    EXPECT_EQ (expectedECInstanceId.GetValue (), actualECInstanceId.GetValue ()) << L"Unexpected ECInstanceId parsed from InstanceId " << instanceId;

    instanceId = "0";
    expectedECInstanceId = ECInstanceId(0LL);
    actualECInstanceId = ECInstanceId ();
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_FALSE (success) << "Unexpected failure of ECInstanceIdHelper::FromString";

    instanceId = "0000";
    expectedECInstanceId = ECInstanceId (0LL);
    actualECInstanceId = ECInstanceId ();
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_FALSE (success) << "Unexpected failure of ECInstanceIdHelper::FromString";

    instanceId = "-123456";
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_TRUE (success) << L"InstanceId with negative number '" << instanceId << L"' is expected to be supported by ECInstanceIdHelper::FromString";
    EXPECT_EQ(actualECInstanceId.GetValue(), -123456LL);

    instanceId = "-12345678901234";
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_TRUE(success) << L"InstanceId with negative number '" << instanceId << L"' is expected to be supported by ECInstanceIdHelper::FromString";
    EXPECT_EQ(actualECInstanceId.GetValue(), -12345678901234LL);

    //now test with invalid instance ids
    BeTest::SetFailOnAssert(false);

    instanceId = "0x75BCD15";
    expectedECInstanceId = ECInstanceId (123456789LL);
    actualECInstanceId = ECInstanceId ();
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_FALSE (success) << L"InstanceId with hex formatted number '" << instanceId << L"' is not expected to be supported by ECInstanceIdHelper::FromString";

    instanceId = "i-12345";
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_FALSE (success) << L"InstanceId starting with i- '" << instanceId << L"' is not expected to be supported by ECInstanceIdHelper::FromString";

    instanceId = "1234a123";
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_FALSE (success) << L"Non-numeric InstanceId '" << instanceId << L"' is not expected to be supported by ECInstanceIdHelper::FromString";

    instanceId = "blabla";
    success = ECInstanceIdHelper::FromString (actualECInstanceId, instanceId);
    EXPECT_FALSE (success) << L"Non-numeric InstanceId '" << instanceId << L"' is not expected to be supported by ECInstanceIdHelper::FromString";

    BeTest::SetFailOnAssert (true);
    }   

TEST (ECDbInstances, CreateRoot_ExistingRoot_ReturnsSameKey_ECDBTEST)
    {
    ECDbTestProject::Initialize ();

    BeFileName schemaDir;
    BeTest::GetHost ().GetDocumentsRoot (schemaDir);
    schemaDir.AppendToPath (L"DgnDb");
    schemaDir.AppendToPath (L"ECDb");
    schemaDir.AppendToPath (L"Schemas");
    BeFileName dsCacheSchema1_4 (nullptr, schemaDir.GetName (), L"DSCacheSchema.01.04.ecschema.xml", nullptr);

    // Setup ECDb
    ECDb db;
    ASSERT_EQ (BE_SQLITE_OK, db.CreateNewDb (":memory:"));

    // Setup Schema
    auto context = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr schema;
    ASSERT_EQ (SchemaReadStatus::SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlFile (schema, dsCacheSchema1_4.GetName (), *context));
    ASSERT_EQ (SUCCESS, db.Schemas ().ImportECSchemas (context->GetCache ()));

    ECClassCP rootClass = db.GetClassLocater ().LocateClass ("DSCacheSchema", "Root");
    ASSERT_NE (nullptr, rootClass);

    // Names
    Utf8String rootName = "Foo";

    // Test quety for same instance
    Utf8String ecsql = "SELECT ECInstanceId FROM [DSC].[Root] WHERE [Name] = ? LIMIT 1 ";
    ECSqlStatement statement;
    ASSERT_EQ (ECSqlStatus::Success, statement.Prepare (db, ecsql.c_str ()));
    ASSERT_EQ (ECSqlStatus::Success, statement.BindText (1, rootName.c_str (), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ (BE_SQLITE_DONE, statement.Step ());

    // Insert one instnace
    Json::Value rootInstance;
    rootInstance["Name"] = rootName;
    rootInstance["Persistence"] = 0;

    JsonInserter inserter (db, *rootClass);
    ASSERT_EQ (SUCCESS, inserter.Insert (rootInstance));

    // Try again
    statement.Reset ();
    statement.ClearBindings ();
    ASSERT_EQ (ECSqlStatus::Success, statement.BindText (1, rootName.c_str (), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ (BE_SQLITE_ROW, statement.Step ());
    EXPECT_EQ (ECInstanceId (1), statement.GetValueId <ECInstanceId> (0));
    }

//TEST (ECDbInstances, ViewTrigger)
//    {
//    ECDbTestProject::Initialize ();
//
//        {
//        printf ("Deleting using ECSql\r\n");
//        ECDb db;
//        db.OpenBeSQLiteDb ("E:/misc/A.ecdb", Db::OpenParams (Db::OpenMode::ReadWrite));
//
//        Statement sp1;
//        sp1.Prepare (db, "SELECT ECInstanceId FROM tt_SP1");
//        std::vector<ECInstanceId> sp1ToBeDeleted;
//        while (sp1.Step () == BE_SQLITE_ROW)
//            sp1ToBeDeleted.push_back (sp1.GetValueId <ECInstanceId> (0));
//        sp1.Finalize ();
//
//
//        ECSqlStatement deleteSP1;
//        deleteSP1.Prepare (db, "DELETE FROM ONLY tt.SP1 WHERE ECInstanceId = ?");
//        StopWatch ecsqlW;
//        ecsqlW.Start ();
//        for (auto ecid : sp1ToBeDeleted)
//            {
//            deleteSP1.Reset ();
//            deleteSP1.ClearBindings ();
//            //printf ("deleting = %lld\r\n", ecid.GetValue ());
//            deleteSP1.BindId (1, ecid);
//            deleteSP1.Step ();
//            }
//        ecsqlW.Stop ();
//        deleteSP1.Finalize ();
//
//        printf ("It took ECSQL Total : %.4lf seconds to delete %d instances of tt.SP1\r\n", ecsqlW.GetElapsedSeconds (), sp1ToBeDeleted.size ());
//        db.SaveChanges ();
//        };
//            {
//            ECDb db;
//            db.OpenBeSQLiteDb ("E:/misc/B.ecdb", Db::OpenParams (Db::OpenMode::ReadWrite));
//            printf ("Deleting using View\r\n");
//            Statement sp1;
//            sp1.Prepare (db, "SELECT ECInstanceId FROM tt_SP1");
//            std::vector<ECInstanceId> sp1ToBeDeleted;
//            while (sp1.Step () == BE_SQLITE_ROW)
//                sp1ToBeDeleted.push_back (sp1.GetValueId <ECInstanceId> (0));
//            sp1.Finalize ();
//
//            Statement deleteSP1;
//            deleteSP1.Prepare (db, "DELETE FROM VC_tt_SP1 WHERE ECInstanceId = ?");
//            StopWatch ecsqlW;
//            ecsqlW.Start ();
//            for (auto ecid : sp1ToBeDeleted)
//                {
//                deleteSP1.Reset ();
//                deleteSP1.ClearBindings ();
//                //printf ("deleting = %lld\r\n", ecid.GetValue ());
//                deleteSP1.BindId (1, ecid);
//                deleteSP1.Step ();
//                }
//            ecsqlW.Stop ();
//            deleteSP1.Finalize ();
//            
//            printf ("It took SQL on View with Trigger Total : %.4lf seconds to delete %d instances of tt.SP1\r\n", ecsqlW.GetElapsedSeconds (), sp1ToBeDeleted.size ());
//            db.SaveChanges ();
//            };
//
//    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, DeleteECInstances)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    // Check all the instances inserted
    ECInstanceMapCR importedInstances = test.GetImportedECInstances();
    for (ECInstanceMap::const_iterator iter = importedInstances.begin(); iter != importedInstances.end(); iter++)
        {
        IECInstancePtr importedInstance = iter->second;
        ECClassCR ecClass = importedInstance->GetClass();
        ECInstanceDeleter deleter (ecdb, ecClass);
        ASSERT_TRUE (deleter.IsValid ());
        BentleyStatus stat = deleter.Delete (*importedInstance);
        ASSERT_EQ(SUCCESS, stat);
        }
    }   
TEST(ECDbInstances, QuoteTest)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    ECSqlStatement stmt1;
    ASSERT_TRUE (stmt1.Prepare (ecdb, "INSERT INTO stco.ClassWithPrimitiveProperties (stringProp) VALUES('''a''a''')") == ECSqlStatus::Success);
    ASSERT_TRUE (stmt1.Step () == BE_SQLITE_DONE);
    ECSqlStatement stmt2;
    ASSERT_TRUE (stmt2.Prepare (ecdb, "SELECT stringProp FROM stco.ClassWithPrimitiveProperties WHERE stringProp = '''a''a'''") == ECSqlStatus::Success);
    ASSERT_TRUE (stmt2.Step () == BE_SQLITE_ROW);
     ECSqlStatement stmt3;
    ASSERT_TRUE (stmt3.Prepare (ecdb, "UPDATE ONLY stco.ClassWithPrimitiveProperties SET stringProp = '''g''''g'''") == ECSqlStatus::Success);
    ASSERT_TRUE (stmt3.Step () == BE_SQLITE_DONE);
     ECSqlStatement stmt4;
    ASSERT_TRUE (stmt4.Prepare (ecdb, "SELECT stringProp FROM stco.ClassWithPrimitiveProperties WHERE stringProp = '''g''''g'''") == ECSqlStatus::Success);
    ASSERT_TRUE (stmt4.Step () == BE_SQLITE_ROW);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PopulatePrimitiveValueWithCustomDataSet (ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    value.Clear();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String  : value.SetUtf8CP("Sample string 2"); break;
        case PRIMITIVETYPE_Integer : value.SetInteger(987); break;
        case PRIMITIVETYPE_Long    : value.SetLong(987654321); break;
        case PRIMITIVETYPE_Double  : value.SetDouble(PI*3);break;
        case PRIMITIVETYPE_DateTime: 
            {
            DateTime dt;
            DateTimeInfo dti;
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecProperty) == ECOBJECTS_STATUS_Success)
                {
                DateTime::Info info = dti.GetInfo (true);
                if (info.GetKind () == DateTime::Kind::Local)
                    {
                    //local date times are not supported by ECObjects
                    break;
                    }

                DateTime::FromJulianDay (dt, 2456341.75, info); 
                }
            else
                {
                DateTime::FromJulianDay (dt, 2456341.75, DateTimeInfo::GetDefault ()); 
                }

            value.SetDateTime (dt); 
            break;
            }
        case PRIMITIVETYPE_Boolean : value.SetBoolean(false); break;
        case PRIMITIVETYPE_Binary  : 
            {
            Byte blob[]= {0x0a, 0x0a, 0x0c, 0x0c, 0x0e, 0x0e, 0x3a, 0xaa, 0xff, 0xb };
            value.SetBinary(blob, 10);
            break;
            }
        case PRIMITIVETYPE_Point2D : 
            {
            DPoint2d point2d;
            point2d.x=33.11;
            point2d.y=44.12;
            value.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D :
            {
            DPoint3d point3d;
            point3d.x=12.33;
            point3d.y=44.54;
            point3d.z=21.55;
            value.SetPoint3D(point3d);
            break;
            }
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Krischan.Eberle                  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SetupInsertECInstanceWithNullValues 
(
ECInstanceKey& instanceKey,
ECValueP nonNullValue,
ECClassCP& testClass,
ECDbTestProject& testProject, 
Utf8CP testClassName,
Utf8CP nonNullPropertyName
)
    {
    ECClassP testClassP = testProject.GetTestSchemaManager ().GetTestSchema ()->GetClassP (testClassName);

    if (!EXPECTED_CONDITION (testClassP != nullptr))
        {
        return ERROR;
        }

    testClass = testClassP;

    ECInstanceInserter inserter (testProject.GetECDb (), *testClass);
    if (!EXPECTED_CONDITION (inserter.IsValid ()))
        {
        return ERROR;
        }

    // Create a new instance with only one prop value being populated (to avoid the special
    // case of a fully empty instance - which might need extra treatment)
    IECInstancePtr testInstance = testProject.CreateECInstance (*testClass);
    testProject.AssignRandomValueToECInstance (nonNullValue, testInstance, nonNullPropertyName);

    auto stat = inserter.Insert (instanceKey, *testInstance);
    BeAssert (stat == SUCCESS);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Krischan.Eberle                  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, InsertECInstancesWithNullValues)
    {
    // Create and populate a sample project
    ECDbTestProject testProject;
    ECDbR db = testProject.Create ("insertwithnullvalues.ecdb", L"ECSqlTest.01.00.ecschema.xml", false);

    Utf8CP const testClassName = "PSA";
    Utf8CP const nonNullPropertyName = "I";

    ECInstanceKey instanceKey;
    ECClassCP testClass = nullptr;
    BentleyStatus setupState = SetupInsertECInstanceWithNullValues (instanceKey, nullptr, testClass, testProject, testClassName, nonNullPropertyName);
    ASSERT_EQ (SUCCESS, setupState);

    Utf8String ecsql ("SELECT * FROM ");
    ecsql.append (ECSqlBuilder::ToECSqlSnippet (*testClass)).append (" WHERE ECInstanceId = ?");
    
    ECSqlStatement statement;
    auto stat = statement.Prepare (db, ecsql.c_str ());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stat = statement.BindId (1, instanceKey.GetECInstanceId ());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    int rowCount = 0;
    while (statement.Step () == BE_SQLITE_ROW)
        {
        ++rowCount;
        const int propCount = statement.GetColumnCount ();
        for (int i = 0; i < propCount; ++i)
            {
            ECPropertyCP prop = statement.GetColumnInfo (i).GetProperty ();
            Utf8StringCR propName = prop->GetName();
            bool expectedIsNull = propName.CompareTo (nonNullPropertyName) != 0 && !propName.Equals (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY);
            if (prop->GetIsArray())
                expectedIsNull = false; // arrays are never Null

            bool actualIsNull = statement.IsValueNull (i);
            EXPECT_EQ (expectedIsNull, actualIsNull) << "ECSqlStatement::IsNullValue failed for property index " << i << " (property name " << propName.c_str () << ")";
            }
        }

    //only one instance was inserted, so row count is expected to be one.
    ASSERT_EQ (1, rowCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Krischan.Eberle                  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, ECInstanceAdapterGetECInstanceWithNullValues)
    {
    // Create and populate a sample project
    ECDbTestProject testProject;
    ECDbR db = testProject.Create ("insertwithnullvalues.ecdb", L"ECSqlTest.01.00.ecschema.xml", false);

    Utf8CP const testClassName = "PSA";
    Utf8CP const nonNullPropertyName = "I";

    ECInstanceKey instanceKey;
    ECClassCP testClass = nullptr;
    BentleyStatus setupState = SetupInsertECInstanceWithNullValues (instanceKey, nullptr, testClass, testProject, testClassName, nonNullPropertyName);
    ASSERT_EQ (SUCCESS, setupState);

    Utf8String ecsql ("SELECT * FROM ");
    ecsql.append (ECSqlBuilder::ToECSqlSnippet (*testClass)).append (" WHERE ECInstanceId = ?");

    ECSqlStatement statement;
    auto stat = statement.Prepare (db, ecsql.c_str ());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stat = statement.BindId (1, instanceKey.GetECInstanceId ());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ECInstanceECSqlSelectAdapter dataAdapter (statement);

    ECValue value;
    int rowCount = 0;
    DbResult result;
    while ((result = statement.Step ()) == BE_SQLITE_ROW)
        {
        ++rowCount;
        IECInstancePtr instance = dataAdapter.GetInstance ();

        ECValuesCollectionPtr propValues = ECValuesCollection::Create (*instance);
        for (ECPropertyValueCR propValue : *propValues)
            {
            Utf8CP propName = propValue.GetValueAccessor ().GetAccessString ();
            const bool expectedIsNull = BeStringUtilities::Stricmp (propName, nonNullPropertyName) != 0;
            value = propValue.GetValue ();
            bool actualIsNull = ECDbTestUtility::IsECValueNull (value);

            EXPECT_EQ (expectedIsNull, actualIsNull) << "Assertion of IsNull check for property value failed for property '" << propName << "'.";
            } 
        }

    //only one instance was inserted, so row count is expected to be one.
    ASSERT_EQ (1, rowCount);
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>08/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
TEST(ECDbInstances, ImportSchemaThenInsertInstances)
    {
    ECDbTestProject test;
    auto& dgndb = test.Create("importecschema.ecdb");
    Utf8String filename = dgndb.GetDbFileName();
    dgndb.CloseDb();
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (filename.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    ASSERT_EQ (SUCCESS, db.Schemas ().ImportECSchemas (schemaContext->GetCache(), 
        ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema should have imported successfully after closing and re-opening the database.";

    ECClassP building = ecSchema->GetClassP("Building");

    auto newInst = ECDbTestProject::CreateArbitraryECInstance (*building, PopulatePrimitiveValueWithCustomDataSet);
    ECInstanceInserter inserter (db, *building);
    ECInstanceKey instanceKey;
    auto insertStatus = inserter.Insert (instanceKey, *newInst);
    ASSERT_EQ (SUCCESS, insertStatus);    

    }

void SetStruct1Values(StandaloneECInstancePtr instance, bool boolVal, int intVal)
    {
    instance->SetValue("Struct1BoolMember", ECValue(boolVal));
    instance->SetValue("Struct1IntMember", ECValue(intVal));
    }

void SetStruct2Values
(
StandaloneECInstancePtr instance, 
Utf8CP stringVal, 
double doubleVal, 
StandaloneECInstancePtr struct1, 
StandaloneECInstancePtr struct2, 
StandaloneECInstancePtr struct3
)
    {
    instance->SetValue("Struct2StringMember", ECValue(stringVal));
    instance->SetValue("Struct2DoubleMember", ECValue(doubleVal));
    instance->AddArrayElements("NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct(struct1.get());
    instance->SetValue("NestedArray", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct(struct2.get());
    instance->SetValue("NestedArray", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct(struct3.get());
    instance->SetValue("NestedArray", structVal3, 2);
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>08/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
TEST(ECDbInstances, CreateAndImportSchemaThenInsertInstance)
    {
    ECDbTestProject test;
    auto& dgndb = test.Create("importecschema.ecdb");
    Utf8String filename = dgndb.GetDbFileName();
    dgndb.CloseDb();
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (filename.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", 1, 2);
    schema->SetNamespacePrefix("ts");
    schema->SetDescription("Schema for testing nested struct arrays");
    schema->SetDisplayLabel("Display Label");

    ECClassP struct1;
    schema->CreateClass(struct1, "Struct1");
    PrimitiveECPropertyP boolProp1;
    struct1->CreatePrimitiveProperty(boolProp1, "Struct1BoolMember", PRIMITIVETYPE_Boolean);
    PrimitiveECPropertyP intProp1;
    struct1->CreatePrimitiveProperty(intProp1, "Struct1IntMember", PRIMITIVETYPE_Integer);
    struct1->SetIsStruct(true);

    ECClassP struct2;
    schema->CreateClass(struct2, "Struct2");
    PrimitiveECPropertyP stringProp2;
    struct2->CreatePrimitiveProperty(stringProp2, "Struct2StringMember", PRIMITIVETYPE_String);
    PrimitiveECPropertyP doubleProp2;
    struct2->CreatePrimitiveProperty(doubleProp2, "Struct2DoubleMember", PRIMITIVETYPE_Double);
    ArrayECPropertyP structArrayProperty2;
    struct2->CreateArrayProperty(structArrayProperty2, "NestedArray", struct1);
    struct2->SetIsStruct(true);

    ECClassP testClass;
    schema->CreateClass(testClass, "TestClass");
    ArrayECPropertyP nestedArrayProperty;
    testClass->CreateArrayProperty(nestedArrayProperty, "StructArray", struct2);

    auto schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema(*schema);
    ASSERT_EQ (SUCCESS, db.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (true, true)));

    StandaloneECEnablerPtr struct1Enabler = struct1->GetDefaultStandaloneEnabler ();
    StandaloneECEnablerPtr struct2Enabler = struct2->GetDefaultStandaloneEnabler ();
    StandaloneECEnablerPtr testClassEnabler = testClass->GetDefaultStandaloneEnabler ();

    ECN::StandaloneECInstancePtr struct1Instance1 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance2 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance3 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance4 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance5 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance6 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance7 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance8 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance9 = struct1Enabler->CreateInstance();

    SetStruct1Values(struct1Instance1, true, 0);
    SetStruct1Values(struct1Instance2, true, 1);
    SetStruct1Values(struct1Instance3, false, 2);
    SetStruct1Values(struct1Instance4, true, 3);
    SetStruct1Values(struct1Instance5, false, 4);
    SetStruct1Values(struct1Instance6, true, 5);
    SetStruct1Values(struct1Instance7, false, 6);
    SetStruct1Values(struct1Instance8, false, 7);
    SetStruct1Values(struct1Instance9, true, 8);

    ECN::StandaloneECInstancePtr struct2Instance1 = struct2Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct2Instance2 = struct2Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct2Instance3 = struct2Enabler->CreateInstance();

    SetStruct2Values(struct2Instance1, "string2-1", 1.001, struct1Instance1, struct1Instance2, struct1Instance3);
    SetStruct2Values(struct2Instance2, "string2-2", 10.002, struct1Instance4, struct1Instance5, struct1Instance6);
    SetStruct2Values(struct2Instance3, "string2-3", 15.003, struct1Instance7, struct1Instance8, struct1Instance9);

    ECN::StandaloneECInstancePtr testClassInstance = testClassEnabler->CreateInstance();
    testClassInstance->AddArrayElements("StructArray", 3);
    ECValue structVal1;
    structVal1.SetStruct(struct2Instance1.get());
    testClassInstance->SetValue("StructArray", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct(struct2Instance2.get());
    testClassInstance->SetValue("StructArray", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct(struct2Instance3.get());
    testClassInstance->SetValue("StructArray", structVal3, 2);

    ECInstanceInserter inserter (db, *testClass);
    ASSERT_TRUE (inserter.IsValid ());
    ECInstanceKey instanceKey;
    auto insertStatus = inserter.Insert (instanceKey, *testClassInstance);
    ASSERT_EQ (SUCCESS, insertStatus);

    SqlPrintfString ecSql ("SELECT StructArray FROM TestSchema.TestClass");
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (db, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == status);

    while (ecStatement.Step() == BE_SQLITE_ROW)
        {
        IECSqlArrayValue const& arrayValue = ecStatement.GetValueArray (0);
        int arrayLength = arrayValue.GetArrayLength ();
        ASSERT_EQ(3, arrayLength);
        int arrayIndex = 0;
        for (IECSqlValue const* arrayElementValue : arrayValue)
            {
            IECSqlStructValue const& structValue = arrayElementValue->GetStruct ();
            auto doubleValue = structValue.GetValue (1).GetDouble ();
            if (arrayIndex == 0)
                ASSERT_EQ(1.001, doubleValue);
            else if (arrayIndex == 1)
                ASSERT_EQ(10.002, doubleValue);
            else
                ASSERT_EQ(15.003, doubleValue);
            IECSqlArrayValue const& nestedArrayValue = structValue.GetValue (2).GetArray ();
            int nestedArrayIndex = 0;
            for (IECSqlValue const* nestedArrayElement : nestedArrayValue)
                {
                auto intValue = nestedArrayElement->GetStruct ().GetValue (1).GetInt ();
                ASSERT_EQ((arrayIndex * 3) + nestedArrayIndex, intValue);
                //printf("Outer array: %d\nInner Array: %d\nIntValue: %d\n\n", arrayIndex, nestedArrayIndex, intValue);
                nestedArrayIndex++;
                }
            arrayIndex++;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald                 08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, UpdateArrayProperty)
    {
    // Create
    ECDbTestProject test;
    ECDbR db = test.Create ("updateArrayProperty.ecdb", L"KitchenSink.01.00.ecschema.xml", false);

    ECN::ECClassCP testClass = db.Schemas ().GetECClass ("KitchenSink", "TestClass");
    ASSERT_TRUE (testClass != nullptr);

    StandaloneECEnablerPtr testEnabler = testClass->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr testInstance = testEnabler->CreateInstance();
    testInstance->AddArrayElements("SmallIntArray", 3);
    ECValue v;
    v.SetInteger(0);
    testInstance->SetValue("SmallIntArray", v, 0);
    v.SetInteger(1);
    testInstance->SetValue("SmallIntArray", v, 1);
    v.SetInteger(2);
    testInstance->SetValue("SmallIntArray", v, 2);

    ECInstanceInserter inserter (db, *testClass);
    ASSERT_TRUE (inserter.IsValid ());
    ECInstanceKey instanceKey;
    auto insertStatus = inserter.Insert (instanceKey, *testInstance);
    ASSERT_EQ (SUCCESS, insertStatus);

    SqlPrintfString ecSql ("SELECT ECInstanceId, GetECClassId() as ECClassId, SmallIntArray FROM KitchenSink.TestClass");
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (db, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == status);
    ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

    ASSERT_TRUE (BE_SQLITE_ROW == ecStatement.Step());
    IECInstancePtr selectedInstance = dataAdapter.GetInstance();
    ASSERT_TRUE (selectedInstance.IsValid());

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == selectedInstance->GetValue (v, "SmallIntArray"));
    ASSERT_TRUE(v.IsArray());
    ArrayInfo info = v.GetArrayInfo();
    ASSERT_EQ(3, info.GetCount());

    selectedInstance->ClearArray("SmallIntArray");
    selectedInstance->AddArrayElements("SmallIntArray", 2);
    v.SetInteger(0);
    selectedInstance->SetValue("SmallIntArray", v, 0);
    v.SetInteger(1);
    selectedInstance->SetValue("SmallIntArray", v, 1);

    ECInstanceUpdater updater(db, *testClass);
    ASSERT_TRUE (updater.IsValid ());
    auto updateStatus = updater.Update(*selectedInstance);
    ASSERT_EQ (SUCCESS, updateStatus);

    ECSqlStatement ecStatement2;
    ECSqlStatus status2 = ecStatement2.Prepare (db, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == status2);
    ECInstanceECSqlSelectAdapter dataAdapter2 (ecStatement2);

    ASSERT_TRUE (BE_SQLITE_ROW == ecStatement2.Step());
    IECInstancePtr updatedInstance = dataAdapter2.GetInstance();
    ASSERT_TRUE (updatedInstance.IsValid());

    ASSERT_TRUE (ECOBJECTS_STATUS_Success == updatedInstance->GetValue (v, "SmallIntArray"));
    ASSERT_TRUE(v.IsArray());
    ArrayInfo info2 = v.GetArrayInfo();
    ASSERT_EQ(2, info2.GetCount()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, UpdateECInstances)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    ECDbR db = test.Create ("updateecinstances.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    // Check all the instances inserted
    ECInstanceMapCR importedInstances = test.GetImportedECInstances();
    for (ECInstanceMap::const_iterator iter = importedInstances.begin(); iter != importedInstances.end(); iter++)
        {
        ECInstanceId importedInstanceId = iter->first;
        IECInstancePtr importedInstance = iter->second;
        ECClassCR ecClass = importedInstance->GetClass();

        SqlPrintfString ecSql ("SELECT * FROM %s.%s WHERE ECInstanceId=%lld", Utf8String(ecClass.GetSchema().GetName()).c_str(), Utf8String(ecClass.GetName()).c_str(), importedInstanceId.GetValue());

        ECSqlStatement ecStatement;
        ECSqlStatus prepareStatus = ecStatement.Prepare (db, ecSql.GetUtf8CP());
        ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);

        ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);
        DbResult stepStatus = ecStatement.Step();
        ASSERT_TRUE(BE_SQLITE_ROW == stepStatus);

        // First, verify that we can read what we wrote
        IECInstancePtr selectedInstance = dataAdapter.GetInstance();
        ASSERT_TRUE (selectedInstance.IsValid());
        bool areEqual = ECDbTestUtility::CompareECInstances (*importedInstance, *selectedInstance);
        if (!areEqual)
            {
            LOG.errorv("Updated ECInstance does not match expectations. Sending both to stdout. For %s, id=%s", 
                importedInstance->GetClass().GetFullName(), importedInstance->GetInstanceId().c_str());
            printf(importedInstance->ToString("IMPORT").c_str()); printf("\n");
            printf(selectedInstance->ToString("SELECT").c_str()); printf("\n");
            ASSERT_TRUE (false && "Selected ECInstance does not match expectations.");
            }

        // See if we can read it directly, too
        ecStatement.Reset();

        stepStatus = ecStatement.Step();
        ASSERT_TRUE(BE_SQLITE_ROW == stepStatus);

        int nColumns = ecStatement.GetColumnCount();
        LOG.infov("Getting values for '%s' (%lld) ECInstanceId=%lld", ecClass.GetFullName(), ecClass.GetId(), importedInstanceId.GetValue());
        for (int i = 0; i < nColumns; i++)
            {
            //EXPECT_TRUE (&ecClass == ecStatement.GetClass(i)); Not true when it is a property from an embedded struct.
            ECPropertyCP ecProperty = ecStatement.GetColumnInfo (i).GetProperty();
            ASSERT_TRUE (nullptr != ecProperty);

            Utf8String propertyAccessString = ecProperty->GetName();//ecStatement.GetPropertyAccessString();

            PrimitiveECPropertyCP primitiveProperty = ecProperty->GetAsPrimitiveProperty();
            if (primitiveProperty)
                {
                auto primType = primitiveProperty->GetType ();
                //DateTime and points are not implicitly convertible to strings, therefore handle them separately.
                if (primType == PRIMITIVETYPE_DateTime)
                    {
                    //There are some DateTime properties in the test schema which are intentionally carrying a corrupt DateTimeInfo CA.
                    //Retrieving values for those throws assertions
                    BeTest::SetFailOnAssert (false);
                    DateTime dt = ecStatement.GetValueDateTime (i);
                    BeTest::SetFailOnAssert (true);
                    LOG.tracev("'%s'='%s'", propertyAccessString.c_str(), dt.ToUtf8String ().c_str ());
                    }
                else if (primType == PRIMITIVETYPE_Point2D)
                    {
                    DPoint2d pt = ecStatement.GetValuePoint2D (i);
                    LOG.tracev("'%s'=(%.4f, %.4f)", propertyAccessString.c_str(), pt.x, pt.y);
                    }
                else if (primType == PRIMITIVETYPE_Point3D)
                    {
                    DPoint3d pt = ecStatement.GetValuePoint3D (i);
                    LOG.tracev("'%s'=(%.4f, %.4f, %.4f)", propertyAccessString.c_str(), pt.x, pt.y, pt.z);
                    }
                else
                    {
                    Utf8CP v = ecStatement.GetValueText (i);
                    int size = 0;
                    const Byte* b = (const Byte *) ecStatement.GetValueBinary(i, &size);
                    Utf8String valueAsString = v ? v : "";
                    LOG.tracev("'%s'='%s',  size=%d, b=0x%lx", propertyAccessString.c_str(), valueAsString.c_str(), size, b);
                    }
                continue;
                }

            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if (arrayProperty)
                {
                IECSqlArrayValue const& arrayValue = ecStatement.GetValueArray (i);
                int arrayLength = arrayValue.GetArrayLength ();
                if (0 >= arrayLength)
                    continue;
                // WIP_ECSQL: Need to actually read the data here?
                continue;
                }
            }

        // Create a new instance with new custom data;
        IECInstancePtr updateInst = test.CreateArbitraryECInstance (importedInstance->GetClass(), PopulatePrimitiveValueWithCustomDataSet);

        // Set instance id we need this for update operation
        updateInst->SetInstanceId (importedInstance->GetInstanceId().c_str());

        ECInstanceUpdater updater (db, ecClass);
        if (ecClass.GetPropertyCount() == 0)
            {
            ASSERT_FALSE(updater.IsValid());
            return;
            }
        
        // update the instance with new data.
        auto updateStatus = updater.Update (*updateInst);
        ASSERT_EQ (SUCCESS, updateStatus);


        // now read back the instance from db
        ecStatement.Reset();
        stepStatus = ecStatement.Step();
        ASSERT_TRUE(BE_SQLITE_ROW == stepStatus);

        IECInstancePtr actual = dataAdapter.GetInstance();
        ASSERT_TRUE (actual.IsValid());
        areEqual = ECDbTestUtility::CompareECInstances (*updateInst, *actual);
        if (!areEqual)
            {
            LOG.errorv("Updated ECInstance does not match expectations. Sending both to stdout. For %s, id=%s", 
                updateInst->GetClass().GetFullName(), updateInst->GetInstanceId().c_str());
            printf(updateInst->ToString("EXPECT").c_str()); printf("\n");
            printf(    actual->ToString("ACTUAL").c_str()); printf("\n");
            EXPECT_TRUE (false && "Updated ECInstance does not match expectations.");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                           07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, FindECInstances)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    IECInstancePtr   resultInstance;
    ECValue          v;
    int              rows;

    ECClassKeys    classKeys;
    db.Schemas().GetECClassKeys (classKeys, "StartupCompany");
    for (ECClassKey ecClassKey : classKeys)
        {
        ECClassCP ecClass = db.Schemas().GetECClass (ecClassKey.GetECClassId());  // WIP_FNV: start taking ECSchemaName, ECClassName, instead
        ASSERT_TRUE (ecClass != nullptr);

        Utf8String className = ecClassKey.GetName();
        if (className == "ClassWithPrimitiveProperties")
            {
            SqlPrintfString ecSql ("SELECT * FROM [StartupCompany].[ClassWithPrimitiveProperties] WHERE intProp > 0");
            ECSqlStatement ecStatement;
            ECSqlStatus status = ecStatement.Prepare (db, ecSql.GetUtf8CP());
            ASSERT_TRUE (ECSqlStatus::Success == status);
            ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

            rows = 0;
            while (ecStatement.Step() == BE_SQLITE_ROW)
                {
                resultInstance = dataAdapter.GetInstance();
                ASSERT_TRUE (resultInstance.IsValid());
                ASSERT_TRUE (ECOBJECTS_STATUS_Success == resultInstance->GetValue (v, "intProp"));
                ASSERT_TRUE (v.GetInteger() > 0);
                rows++;
                }
            LOG.infov(L"Returned %d rows for '(intProp > 0)'", rows);
            ASSERT_EQ (rows, 3);
            }
        else if(className == "AAA")
            {
            SqlPrintfString ecSql ("SELECT * FROM [StartupCompany].[AAA] WHERE l = 123456789");
            ECSqlStatement ecStatement;
            ASSERT_TRUE (ECSqlStatus::Success == ecStatement.Prepare (db, ecSql.GetUtf8CP()));
            ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

            rows = 0;
            while (ecStatement.Step() == BE_SQLITE_ROW)
                {
                resultInstance = dataAdapter.GetInstance();
                ASSERT_TRUE (resultInstance.IsValid());
                ASSERT_TRUE (ECOBJECTS_STATUS_Success == resultInstance->GetValue (v, "l"));
                ASSERT_TRUE (v.GetLong() == 123456789L);
                rows++;
                }
            LOG.infov(L"Returned %d rows for '(l = 123456789)'", rows);
            ASSERT_EQ (rows, 3);
            }
        else if(className == "AAFoo")
            {
            SqlPrintfString ecSql ("SELECT * FROM ONLY [StartupCompany].[AAFoo] WHERE doubleAAFoo BETWEEN 0 AND 100");
            ECSqlStatement ecStatement;
            ASSERT_TRUE (ECSqlStatus::Success == ecStatement.Prepare (db, ecSql.GetUtf8CP()));
            ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

            rows = 0;
            while (ecStatement.Step() == BE_SQLITE_ROW)
                {
                resultInstance = dataAdapter.GetInstance();
                ASSERT_TRUE (resultInstance.IsValid());
                ASSERT_TRUE (ECOBJECTS_STATUS_Success == resultInstance->GetValue (v, "doubleAAFoo"));
                ASSERT_TRUE (v.GetDouble() >= 0 && v.GetDouble() <= 100);
                rows++;
                }
            LOG.infov(L"Returned %d rows for '(doubleAAFoo BETWEEN 0 AND 100)' ", rows);
            ASSERT_EQ (rows, 3);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, FindECInstancesFromSelectWithMultipleClasses)
    {
    // Save a test project
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    bvector<IECInstancePtr> instances;
    ASSERT_EQ(SUCCESS, testProject.GetInstances(instances, "Foo"));

    IECInstancePtr sourceInstance = instances[0];

    ASSERT_EQ(SUCCESS, testProject.GetInstances(instances, "Bar"));

    IECInstancePtr targetInstance = instances[0];

    ECSchemaPtr schema = testProject.GetTestSchemaManager().GetTestSchema();
    ASSERT_TRUE(schema != nullptr) << "ECDbTestSchemaManager::GetTestSchema returned null";

    ECRelationshipClassCP relClass = schema->GetClassP("Foo_has_Bars")->GetRelationshipClassCP();
    ASSERT_TRUE(relClass != nullptr) << "Could not find relationship class Foo_has_Bars in test schema";

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();
    ASSERT_TRUE(relationshipInstance.IsValid());
    relationshipInstance->SetSource(sourceInstance.get());
    relationshipInstance->SetTarget(targetInstance.get());
    relationshipInstance->SetInstanceId("source->target");

    ECInstanceInserter inserter(ecdb, *relClass);
    ASSERT_TRUE(inserter.IsValid());
    auto insertStatus = inserter.Insert(*relationshipInstance);
    ASSERT_EQ(SUCCESS, insertStatus);

    ECSqlStatement ecStatement;
    ASSERT_TRUE(ECSqlStatus::Success == ecStatement.Prepare(ecdb, "SELECT c0.intFoo, c1.stringBar from [StartupCompany].[Foo] c0 join [StartupCompany].[Bar] c1 using [StartupCompany].[Foo_has_Bars]"));

    ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);
    int rows = 0;
    ECValue v;
    IECInstancePtr resultInstance = nullptr;
    while (ecStatement.Step() == BE_SQLITE_ROW)
        {
        BeTest::SetFailOnAssert (false);
        resultInstance = dataAdapter.GetInstance();
        BeTest::SetFailOnAssert (true);
        ASSERT_TRUE (resultInstance == nullptr);
        rows++;
        }
    ASSERT_TRUE(rows > 0) << "Should have found at least one Foo instance";

    ecStatement.Reset();

    rows = 0;
    ECClassCP ecClass = schema->GetClassP ("Bar");
    ASSERT_TRUE (ecClass != nullptr) << "ECDbTestSchemaManager::GetClassP returned null";
    while (ecStatement.Step() == BE_SQLITE_ROW)
        {
        resultInstance = dataAdapter.GetInstance(ecClass->GetId());
        ASSERT_TRUE (resultInstance.IsValid());
        ASSERT_TRUE (ECOBJECTS_STATUS_Success == resultInstance->GetValue (v, "stringBar"));
        ASSERT_FALSE (v.IsNull ());
        rows++;
        }
    ASSERT_TRUE(rows > 0) << "Should have found at least one Bar instance";

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                           07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, SelectClause)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    ECClassCP employee = db.Schemas().GetECClass("StartupCompany", "Employee");
    ASSERT_TRUE (employee != nullptr);

    ECSqlStatement ecStatement;

    Utf8String jobTitle1;
    int managerId1;
        {
        // ECSQL should honor the order of the ecColumns from the select clause
        ASSERT_TRUE (ECSqlStatus::Success == ecStatement.Prepare (db, "SELECT JobTitle, ManagerID FROM [StartupCompany].[Employee]"));
        ASSERT_TRUE (BE_SQLITE_ROW == ecStatement.Step());
        jobTitle1  = ecStatement.GetValueText(0);
        managerId1 = ecStatement.GetValueInt(1);
        EXPECT_TRUE (ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("JobTitle"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ManagerID"));
        ecStatement.Finalize();
        }

        {
        ASSERT_TRUE (ECSqlStatus::Success == ecStatement.Prepare (db, "SELECT JobTitle, ManagerID FROM [StartupCompany].[Employee]"));
        ASSERT_TRUE (BE_SQLITE_ROW == ecStatement.Step());
        Utf8String jobTitle2  = ecStatement.GetValueText(0);
        int        managerId2 = ecStatement.GetValueInt(1);
        EXPECT_TRUE (ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("JobTitle"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ManagerID"));
        ecStatement.Finalize();

        ASSERT_EQ (managerId1, managerId2);
        ASSERT_TRUE (jobTitle1.Equals(jobTitle2));
        }

        {
        // ECSQL SelectAll (aka '*') should select in same order as ECProperties appear in the ECSchema
        ASSERT_TRUE (ECSqlStatus::Success == ecStatement.Prepare (db, "SELECT * FROM [StartupCompany].[Employee]"));
        ASSERT_TRUE (BE_SQLITE_ROW == ecStatement.Step());
        EXPECT_TRUE (ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("ECInstanceId"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("EmployeeID"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(2).GetProperty()->GetName().Equals("FirstName" ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(3).GetProperty()->GetName().Equals("JobTitle"  ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(4).GetProperty()->GetName().Equals("LastName"  ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(5).GetProperty()->GetName().Equals("ManagerID" ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(6).GetProperty()->GetName().Equals("Room"      ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(7).GetProperty()->GetName().Equals("SSN"       ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(8).GetProperty()->GetName().Equals("Project"   ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(9).GetProperty()->GetName().Equals("WorkPhone" ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(10).GetProperty()->GetName().Equals("MobilePhone"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(11).GetProperty()->GetName().Equals("FullName" ));
        EXPECT_TRUE (ecStatement.GetColumnInfo(12).GetProperty()->GetName().Equals("Certifications"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(13).GetProperty()->GetName().Equals("Location"));
        EXPECT_TRUE (ecStatement.GetValueStruct(13).GetValue (0).GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Coordinate"));
        EXPECT_TRUE (ecStatement.GetValueStruct (13).GetValue (1).GetColumnInfo ().GetProperty ()->GetName ().Equals (/*Location.*/"Street"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(14).GetProperty()->GetName().Equals("EmployeeType"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(15).GetProperty()->GetName().Equals("Address"));
        EXPECT_TRUE (ecStatement.GetValueStruct (15).GetValue (0).GetColumnInfo ().GetProperty ()->GetName ().Equals (/*Location.*/"Coordinate"));
        EXPECT_TRUE (ecStatement.GetValueStruct (15).GetValue (1).GetColumnInfo ().GetProperty ()->GetName ().Equals (/*Location.*/"Street"));
        EXPECT_TRUE (ecStatement.GetColumnInfo(16).GetProperty()->GetName().Equals("EmployeeRecordKey"));

        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbInstances, DeleteWithRelationshipBetweenStructs)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create("structs.ecdb");

    ECSchemaPtr structSchema;
    ECSchema::CreateSchema(structSchema, "StructSchema", 1, 0);
    structSchema->SetNamespacePrefix("ss");
    structSchema->SetDescription("Schema with struct classes and a relationship between them");
    structSchema->SetDisplayLabel("Struct Schema");

    ECClassP struct1;
    structSchema->CreateClass (struct1, "Struct1");
    ECClassP struct2;
    structSchema->CreateClass (struct2, "Struct2");
    ECRelationshipClassP relationshipClass;
    structSchema->CreateRelationshipClass (relationshipClass, "StructToStruct");

    struct1->SetIsStruct(true);
    struct1->SetIsDomainClass(true);
    struct2->SetIsStruct(true);
    struct2->SetIsDomainClass(true);
    
    ASSERT_TRUE(struct1->GetIsStruct());
    ASSERT_TRUE(struct1->GetIsDomainClass());

    ASSERT_TRUE(struct2->GetIsStruct());
    ASSERT_TRUE(struct2->GetIsDomainClass());

    PrimitiveECPropertyP stringProp1;
    struct1->CreatePrimitiveProperty (stringProp1, "StringMember1");
    PrimitiveECPropertyP stringProp2;
    struct2->CreatePrimitiveProperty (stringProp2, "StringMember2");

    relationshipClass->GetSource().AddClass(*struct1);
    relationshipClass->GetSource().SetCardinality(RelationshipCardinality::ZeroMany());
    relationshipClass->GetTarget().AddClass(*struct2);
    relationshipClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());

    // import
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*structSchema);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false)));

    auto struct1Inst = ECDbTestProject::CreateArbitraryECInstance(*struct1, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    ECInstanceInserter inserter1 (ecdb, *struct1);
    ASSERT_TRUE (inserter1.IsValid ());
    auto insertStatus = inserter1.Insert (*struct1Inst);

    auto struct2Inst = ECDbTestProject::CreateArbitraryECInstance(*struct2, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    ECInstanceInserter inserter2 (ecdb, *struct2);
    ASSERT_TRUE (inserter2.IsValid ());
    insertStatus = inserter2.Insert (*struct2Inst);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relationshipClass);
    ASSERT_TRUE (relationshipEnabler.IsValid());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (struct1Inst.get());
    relationshipInstance->SetTarget (struct2Inst.get());
    relationshipInstance->SetInstanceId ("source->target");
    
    ECInstanceInserter inserter3 (ecdb, *relationshipClass);
    ASSERT_TRUE (inserter3.IsValid ());
    insertStatus = inserter3.Insert (*relationshipInstance);

    ECInstanceDeleter deleter1 (ecdb, *struct1);
    auto deleteStatus = deleter1.Delete (*struct1Inst);
    ASSERT_EQ (SUCCESS, deleteStatus);

    }

//---------------------------------------------------------------------------------------
// Test for TFS 112251, the Adapter should check for the class before operation
// @bsimethod                                   Majd.Uddin                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, AdapterCheckClassBeforeOperation)
    {
    ECDbTestProject saveTestProject;
    saveTestProject.Create("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb(saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, stat);

    //Get two classes and create instance of second
    ECClassCP employee = db.Schemas().GetECClass("StartupCompany", "Employee");
    ASSERT_TRUE (employee != nullptr);

    ECClassCP project = db.Schemas().GetECClass("StartupCompany", "Project");
    ASSERT_TRUE (project != nullptr);
    IECInstancePtr instance;
    instance = ECDbTestProject::CreateArbitraryECInstance(*project, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);

    //ECInstance Adapters
    ECInstanceInserter inserter(db, *employee);
    ECInstanceKey instanceKey;
    auto sms = inserter.Insert(instanceKey, *instance);
    EXPECT_EQ(ERROR, sms);

    ECInstanceUpdater updater(db, *employee);
    sms = updater.Update(*instance);
    EXPECT_EQ(ERROR, sms);

    ECInstanceDeleter deleter(db, *employee);
    sms = deleter.Delete(*instance);
    EXPECT_EQ(ERROR, sms);

    //Json Adapters
    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"DgnDb");
    jsonInputFile.AppendToPath(L"FieldEngineerStructArray.json");

    BeTest::SetFailOnAssert(false);
    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ReadJsonInputFromFile(jsonInput, jsonInputFile);

    JsonInserter jsonInserter(db, *employee);
    sms = jsonInserter.Insert(instanceKey, jsonInput);
    EXPECT_EQ(ERROR, sms);

    JsonUpdater jsonUpdater(db, *employee);
    sms = jsonUpdater.Update(jsonInput);
    EXPECT_EQ(ERROR, sms);

    JsonDeleter jsonDeleter(db, *employee);
    ECInstanceId instanceId;
    ECInstanceIdHelper::FromString(instanceId, instance->GetInstanceId().c_str());
    sms = jsonDeleter.Delete(instanceId);
    //ECDB_RowCount
    //EXPECT_EQ(ERROR, sms);

    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// Test for TFS 99872, inserting classes that are Domain, Custom Attribute and Struct
// @bsimethod                                   Majd.Uddin                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, DomainCustomAttributeStructCombinations)
{
    ECDbTestProject saveTestProject;
    saveTestProject.Create("ClassCombinations.ecdb", L"TryClassCombinations.01.00.ecschema.xml", true);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb(saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, stat);
    
    //Trying all combinations where IsDomain is True. IsDomain False are Abstract classes and are not instantiated.
    ECClassCP allTrue = db.Schemas().GetECClass("TryClassCombinations", "S_T_CA_T_D_T");
    ASSERT_TRUE (allTrue != nullptr);
    IECInstancePtr instance;
    instance = ECDbTestProject::CreateArbitraryECInstance(*allTrue, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    ECInstanceInserter inserter(db, *allTrue);
    ECInstanceKey instanceKey;
    auto sms = inserter.Insert(instanceKey, *instance);
    EXPECT_EQ(SUCCESS, sms);

    ECClassCP Test1 = db.Schemas().GetECClass("TryClassCombinations", "S_T_CA_F_D_T");
    ASSERT_TRUE (Test1 != nullptr);
    instance = ECDbTestProject::CreateArbitraryECInstance (*Test1, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    ECInstanceInserter inserter2(db, *Test1);
    sms = inserter2.Insert(instanceKey, *instance);
    EXPECT_EQ(SUCCESS, sms);

    ECClassCP Test2 = db.Schemas().GetECClass("TryClassCombinations", "S_F_CA_T_D_T");
    ASSERT_TRUE (Test2 != nullptr);
    instance = ECDbTestProject::CreateArbitraryECInstance (*Test2, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    ECInstanceInserter inserter3(db, *Test2);
    sms = inserter3.Insert(instanceKey, *instance);
    EXPECT_EQ(SUCCESS, sms);
}

END_ECDBUNITTESTS_NAMESPACE
