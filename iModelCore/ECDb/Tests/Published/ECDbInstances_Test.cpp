/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbInstances_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"
#include <limits>
#include <initializer_list>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECDbInstances : ECDbTestFixture {};

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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                     09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbInstances, CreateRoot_ExistingRoot_ReturnsSameKey_ECDBTEST)
    {
    ECDbTestFixture::Initialize ();

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
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlFile (schema, dsCacheSchema1_4.GetName (), *context));
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

TEST_F(ECDbInstances, QuoteTest)
    {
    ECDb& ecdb = SetupECDb ("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));

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
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecProperty) == ECObjectsStatus::Success)
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
ECDbCR ecdb,
Utf8CP testSchemaName,
Utf8CP testClassName,
Utf8CP nonNullPropertyName
)
    {
    testClass = ecdb.Schemas().GetECClass (testSchemaName, testClassName);
    if (testClass == nullptr)
        return ERROR;

    ECInstanceInserter inserter (ecdb, *testClass);
    if (!inserter.IsValid ())
        return ERROR;

    // Create a new instance with only one prop value being populated (to avoid the special
    // case of a fully empty instance - which might need extra treatment)
    StandaloneECEnablerP instanceEnabler = testClass->GetDefaultStandaloneEnabler();
    IECInstancePtr testInstance = instanceEnabler->CreateInstance();
    EXPECT_TRUE(testInstance != nullptr);

    ECDbTestUtility::AssignRandomValueToECInstance (nonNullValue, testInstance, nonNullPropertyName);

    auto stat = inserter.Insert (instanceKey, *testInstance);
    BeAssert (stat == SUCCESS);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Krischan.Eberle                  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbInstances, InsertECInstancesWithNullValues)
    {
    ECDb& db = SetupECDb ("insertwithnullvalues.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    Utf8CP const testClassName = "PSA";
    Utf8CP const nonNullPropertyName = "I";

    ECInstanceKey instanceKey;
    ECClassCP testClass = nullptr;
    BentleyStatus setupState = SetupInsertECInstanceWithNullValues (instanceKey, nullptr, testClass, db, "ECSqlTest", testClassName, nonNullPropertyName);
    ASSERT_EQ (SUCCESS, setupState);

    Utf8String ecsql ("SELECT * FROM ");
    ecsql.append (testClass->GetECSqlName()).append (" WHERE ECInstanceId = ?");
    
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
TEST_F(ECDbInstances, ECInstanceAdapterGetECInstanceWithNullValues)
    {
    ECDb& db = SetupECDb ("insertwithnullvalues.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    Utf8CP const testClassName = "PSA";
    Utf8CP const nonNullPropertyName = "I";

    ECInstanceKey instanceKey;
    ECClassCP testClass = nullptr;
    BentleyStatus setupState = SetupInsertECInstanceWithNullValues (instanceKey, nullptr, testClass, db, "ECSqlTest", testClassName, nonNullPropertyName);
    ASSERT_EQ (SUCCESS, setupState);

    Utf8String ecsql ("SELECT * FROM ");
    ecsql.append (testClass->GetECSqlName()).append (" WHERE ECInstanceId = ?");

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
TEST_F(ECDbInstances, CreateAndImportSchemaThenInsertInstance)
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

    ECStructClassP struct1;
    schema->CreateStructClass(struct1, "Struct1");
    PrimitiveECPropertyP boolProp1;
    struct1->CreatePrimitiveProperty(boolProp1, "Struct1BoolMember", PRIMITIVETYPE_Boolean);
    PrimitiveECPropertyP intProp1;
    struct1->CreatePrimitiveProperty(intProp1, "Struct1IntMember", PRIMITIVETYPE_Integer);

    ECStructClassP struct2;
    schema->CreateStructClass(struct2, "Struct2");
    PrimitiveECPropertyP stringProp2;
    struct2->CreatePrimitiveProperty(stringProp2, "Struct2StringMember", PRIMITIVETYPE_String);
    PrimitiveECPropertyP doubleProp2;
    struct2->CreatePrimitiveProperty(doubleProp2, "Struct2DoubleMember", PRIMITIVETYPE_Double);
    StructArrayECPropertyP structArrayProperty2;
    struct2->CreateStructArrayProperty(structArrayProperty2, "NestedArray", struct1);

    ECEntityClassP testClass;
    schema->CreateEntityClass(testClass, "TestClass");
    StructArrayECPropertyP nestedArrayProperty;
    testClass->CreateStructArrayProperty(nestedArrayProperty, "StructArray", struct2);

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
TEST_F(ECDbInstances, UpdateArrayProperty)
    {
    ECDb&db = SetupECDb ("updateArrayProperty.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));

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

    Utf8CP ecSql = "SELECT ECInstanceId, GetECClassId() as ECClassId, SmallIntArray FROM KitchenSink.TestClass";
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (db, ecSql);
    ASSERT_TRUE (ECSqlStatus::Success == status);
    ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

    ASSERT_TRUE (BE_SQLITE_ROW == ecStatement.Step());
    IECInstancePtr selectedInstance = dataAdapter.GetInstance();
    ASSERT_TRUE (selectedInstance.IsValid());

    ASSERT_TRUE (ECObjectsStatus::Success == selectedInstance->GetValue (v, "SmallIntArray"));
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
    ECSqlStatus status2 = ecStatement2.Prepare (db, ecSql);
    ASSERT_TRUE (ECSqlStatus::Success == status2);
    ECInstanceECSqlSelectAdapter dataAdapter2 (ecStatement2);

    ASSERT_TRUE (BE_SQLITE_ROW == ecStatement2.Step());
    IECInstancePtr updatedInstance = dataAdapter2.GetInstance();
    ASSERT_TRUE (updatedInstance.IsValid());

    ASSERT_TRUE (ECObjectsStatus::Success == updatedInstance->GetValue (v, "SmallIntArray"));
    ASSERT_TRUE(v.IsArray());
    ArrayInfo info2 = v.GetArrayInfo();
    ASSERT_EQ(2, info2.GetCount()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                           07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbInstances, FindECInstances)
    {
    ECDb& db = SetupECDb ("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), 3);

    IECInstancePtr resultInstance = nullptr;
    ECValue v;
    int rows;

    ECClassKeys classKeys;
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
                ASSERT_TRUE (ECObjectsStatus::Success == resultInstance->GetValue (v, "intProp"));
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
                ASSERT_TRUE (ECObjectsStatus::Success == resultInstance->GetValue (v, "l"));
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
                ASSERT_TRUE (ECObjectsStatus::Success == resultInstance->GetValue (v, "doubleAAFoo"));
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
TEST_F(ECDbInstances, FindECInstancesFromSelectWithMultipleClasses)
    {
    ECDb& ecdb = SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), 3);

    bvector<IECInstancePtr> instances;
    ASSERT_EQ(SUCCESS, GetInstances(instances, "StartupCompany", "Foo"));

    IECInstancePtr sourceInstance = instances[0];

    ASSERT_EQ(SUCCESS, GetInstances(instances, "StartupCompany", "Bar"));

    IECInstancePtr targetInstance = instances[0];

    ECRelationshipClassCP relClass = ecdb.Schemas().GetECClass("StartupCompany", "Foo_has_Bars")->GetRelationshipClassCP();
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
    ECClassCP ecClass = ecdb.Schemas().GetECClass("StartupCompany", "Bar");
    ASSERT_TRUE (ecClass != nullptr) << "ECDbTestSchemaManager::GetClassP returned null";
    while (ecStatement.Step() == BE_SQLITE_ROW)
        {
        resultInstance = dataAdapter.GetInstance(ecClass->GetId());
        ASSERT_TRUE (resultInstance.IsValid());
        ASSERT_TRUE (ECObjectsStatus::Success == resultInstance->GetValue (v, "stringBar"));
        ASSERT_FALSE (v.IsNull ());
        rows++;
        }
    ASSERT_TRUE(rows > 0) << "Should have found at least one Bar instance";

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                           07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbInstances, SelectClause)
    {
    ECDb& db = SetupECDb ("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), 3);

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
// Test for TFS 112251, the Adapter should check for the class before operation
// @bsimethod                                   Majd.Uddin                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbInstances, AdapterCheckClassBeforeOperation)
    {
    ECDb& db = SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));


    //Get two classes and create instance of second
    ECClassCP employee = db.Schemas().GetECClass("StartupCompany", "Employee");
    ASSERT_TRUE (employee != nullptr);

    ECClassCP project = db.Schemas().GetECClass("StartupCompany", "Project");
    ASSERT_TRUE (project != nullptr);
    IECInstancePtr instance;
    instance = ECDbTestUtility::CreateArbitraryECInstance(*project, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);

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
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"FieldEngineerStructArray.json");

    BeTest::SetFailOnAssert(false);
    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ASSERT_EQ(SUCCESS, ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile));

    JsonInserter jsonInserter(db, *employee);
    sms = jsonInserter.Insert(instanceKey, jsonInput);
    EXPECT_EQ(ERROR, sms);

    JsonUpdater jsonUpdater(db, *employee);
    sms = jsonUpdater.Update(instanceKey.GetECInstanceId(), jsonInput);
    EXPECT_EQ(ERROR, sms);

    JsonDeleter jsonDeleter(db, *employee);
    ECInstanceId instanceId;
    ECInstanceIdHelper::FromString(instanceId, instance->GetInstanceId().c_str());
    sms = jsonDeleter.Delete(instanceId);
    //ECDB_RowCount
    //EXPECT_EQ(ERROR, sms);

    BeTest::SetFailOnAssert(true);
    }

END_ECDBUNITTESTS_NAMESPACE
