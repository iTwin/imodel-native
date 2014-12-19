/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/NonPublished/ECDb/ECDbInstances_Test_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include <limits>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

extern void PopulatePrimitiveValueWithCustomDataSet (ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty);
extern void SetStruct1Values (StandaloneECInstancePtr instance, bool boolVal, int intVal);
extern void SetStruct2Values
(
StandaloneECInstancePtr instance,
WCharCP stringVal,
double doubleVal,
StandaloneECInstancePtr struct1,
StandaloneECInstancePtr struct2,
StandaloneECInstancePtr struct3
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances_Old, DeleteECInstances)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    auto const& ecDbStore = ecdb.GetEC();

    // Check all the instances inserted
    ECInstanceMapCR importedInstances = test.GetImportedECInstances();
    for (ECInstanceMap::const_iterator iter = importedInstances.begin(); iter != importedInstances.end(); iter++)
        {
        ECInstanceId importedInstanceId = iter->first;
        IECInstancePtr importedInstance = iter->second;
        ECClassCR ecClass = importedInstance->GetClass();
        ECPersistencePtr persistence = ecDbStore.GetECPersistence (nullptr, ecClass);
        ASSERT_TRUE (persistence.IsValid());
        DeleteStatus deleteStatus = persistence->Delete (importedInstanceId);
        ASSERT_EQ   (DELETE_Success, deleteStatus);
        }
    }   


BentleyStatus SetupInsertECInstanceWithNullValues_Old
(
ECInstanceId& instanceId,
ECValueP nonNullValue,
ECClassCP& testClass,
ECDbTestProject& testProject,
WCharCP testClassName,
WCharCP nonNullPropertyName
)
    {
    ECClassP testClassP = testProject.GetTestSchemaManager ().GetTestSchema ()->GetClassP (testClassName);

    if (!EXPECTED_CONDITION (testClassP != nullptr))
        {
        return ERROR;
        }

    testClass = testClassP;

    ECPersistencePtr persistence = testProject.GetECDb ().GetEC ().GetECPersistence (nullptr, *testClass);
    if (!EXPECTED_CONDITION (persistence.IsValid ()))
        {
        return ERROR;
        }

    // Create a new instance with only one prop value being populated (to avoid the special
    // case of a fully empty instance - which might need extra treatment)
    IECInstancePtr testInstance = testProject.CreateECInstance (*testClass);
    testProject.AssignRandomValueToECInstance (nonNullValue, testInstance, nonNullPropertyName);

    InsertStatus insertStatus = persistence->Insert (&instanceId, *testInstance);

    POSTCONDITION (INSERT_Success == insertStatus, ERROR);
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Krischan.Eberle                  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances_Old, InsertECInstancesWithNullValues)
    {
    // Create and populate a sample project
    ECDbTestProject testProject;
    ECDbR db = testProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    WCharCP const testClassName = L"AAFoo";
    Utf8CP const nonNullPropertyName = "FooTag";
    WString const nonNullPropertyNameW (nonNullPropertyName, BentleyCharEncoding::Utf8);

    ECInstanceId instanceId;
    ECClassCP testClass = nullptr;
    BentleyStatus setupState = SetupInsertECInstanceWithNullValues_Old (instanceId, nullptr, testClass, testProject, testClassName, nonNullPropertyNameW.c_str ());
    ASSERT_EQ (SUCCESS, setupState);

    ECSqlSelectBuilder ecsqlBuilder;
    SqlPrintfString whereStr("ECInstanceId=%lld", instanceId.GetValue());
    ecsqlBuilder.From (*testClass).SelectAll ().Where (whereStr.GetUtf8CP());
    
    ECSqlStatement statement;
    auto stat = statement.Prepare (db, ecsqlBuilder.ToString ().c_str ());
    ASSERT_EQ((int) ECSqlStatus::Success, (int) stat);

    int rowCount = 0;
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        ++rowCount;
        const int propCount = statement.GetColumnCount ();
        for (int i = 0; i < propCount; ++i)
            {
            ECPropertyCP prop = statement.GetColumnInfo (i).GetProperty ();
            WStringCR propName = prop->GetName();
            bool expectedIsNull = propName.CompareTo (nonNullPropertyNameW.c_str ()) != 0 && !propName.Equals (WString (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY, BentleyCharEncoding::Utf8));
            if (prop->GetIsArray())
                expectedIsNull = false; // arrays are never Null

            bool actualIsNull = statement.IsValueNull (i);
            EXPECT_EQ (expectedIsNull, actualIsNull) << L"ECSqlStatement::IsNullValue failed for property index " << i << " (property name " << Utf8String (propName).c_str () << ")";
            }
        }

    //only one instance was inserted, so row count is expected to be one.
    ASSERT_EQ (1, rowCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Krischan.Eberle                  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances_Old, ECInstanceAdapterGetECInstanceWithNullValues)
    {
    // Create and populate a sample project
    ECDbTestProject testProject;
    ECDbR db = testProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    WCharCP const testClassName = L"AAFoo";
    Utf8CP const nonNullPropertyName = "FooTag";
    WString const nonNullPropertyNameW (nonNullPropertyName, BentleyCharEncoding::Utf8);

    ECInstanceId instanceId;
    ECClassCP testClass = nullptr;
    BentleyStatus setupState = SetupInsertECInstanceWithNullValues_Old (instanceId, nullptr, testClass, testProject, testClassName, nonNullPropertyNameW.c_str ());
    ASSERT_EQ (SUCCESS, setupState);

    SqlPrintfString ecSql ("SELECT * FROM %s.%s WHERE ECInstanceId=%lld", Utf8String(testClass->GetSchema().GetName()).c_str(), Utf8String(testClass->GetName()).c_str(), instanceId.GetValue());
    
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (db, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);

    ECInstanceECSqlSelectAdapter dataAdapter (statement);

    ECValue value;
    int rowCount = 0;
    ECSqlStepStatus result;
    while ((result = statement.Step ()) == ECSqlStepStatus::HasRow)
        {
        ++rowCount;
        IECInstancePtr instance = dataAdapter.GetInstance ();

        ECValuesCollectionPtr propValues = ECValuesCollection::Create (*instance);
        for (ECPropertyValueCR propValue : *propValues)
            {
            WCharCP propName = propValue.GetValueAccessor ().GetAccessString ();
            const bool expectedIsNull = BeStringUtilities::Wcsicmp (propName, nonNullPropertyNameW.c_str ()) != 0;
            value = propValue.GetValue ();
            bool actualIsNull = ECDbTestUtility::IsECValueNull (value);

            EXPECT_EQ (expectedIsNull, actualIsNull) << L"Assertion of IsNull check for property value failed for property '" << propName << L"'.";
            } 
        }

    //only one instance was inserted, so row count is expected to be one.
    ASSERT_EQ (1, rowCount);
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>08/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
TEST(ECDbInstances_Old, ImportSchemaThenInsertInstances)
    {
    ECDbTestProject test;
    auto& dgndb = test.Create("importecschema.ecdb");
    Utf8String filename = dgndb.GetDbFileName();
    dgndb.CloseDb();
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (filename.c_str(), Db::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    ASSERT_EQ (SUCCESS, db.GetEC ().GetSchemaManager ().ImportECSchemas (schemaContext->GetCache(), 
        ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema should have imported successfully after closing and re-opening the database.";

    ECClassP building = ecSchema->GetClassP(L"Building");

    auto newInst = ECDbTestProject::CreateArbitraryECInstance (*building, PopulatePrimitiveValueWithCustomDataSet);
    auto persistence = db.GetEC().GetECPersistence (nullptr, *building);
    auto insertStatus = persistence->Insert (nullptr, *newInst);
    ASSERT_EQ (INSERT_Success, insertStatus);    

    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>08/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
TEST(ECDbInstances_Old, CreateAndImportSchemaThenInsertInstance)
    {
    ECDbTestProject test;
    auto& dgndb = test.Create("importecschema.ecdb");
    Utf8String filename = dgndb.GetDbFileName();
    dgndb.CloseDb();
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (filename.c_str(), Db::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, L"TestSchema", 1, 2);
    schema->SetNamespacePrefix(L"ts");
    schema->SetDescription(L"Schema for testing nested struct arrays");
    schema->SetDisplayLabel(L"Display Label");

    ECClassP struct1;
    schema->CreateClass(struct1, L"Struct1");
    PrimitiveECPropertyP boolProp1;
    struct1->CreatePrimitiveProperty(boolProp1, L"Struct1BoolMember", PRIMITIVETYPE_Boolean);
    PrimitiveECPropertyP intProp1;
    struct1->CreatePrimitiveProperty(intProp1, L"Struct1IntMember", PRIMITIVETYPE_Integer);
    struct1->SetIsStruct(true);

    ECClassP struct2;
    schema->CreateClass(struct2, L"Struct2");
    PrimitiveECPropertyP stringProp2;
    struct2->CreatePrimitiveProperty(stringProp2, L"Struct2StringMember", PRIMITIVETYPE_String);
    PrimitiveECPropertyP doubleProp2;
    struct2->CreatePrimitiveProperty(doubleProp2, L"Struct2DoubleMember", PRIMITIVETYPE_Double);
    ArrayECPropertyP structArrayProperty2;
    struct2->CreateArrayProperty(structArrayProperty2, L"NestedArray", struct1);
    struct2->SetIsStruct(true);

    ECClassP testClass;
    schema->CreateClass(testClass, L"TestClass");
    ArrayECPropertyP nestedArrayProperty;
    testClass->CreateArrayProperty(nestedArrayProperty, L"StructArray", struct2);

    auto schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema(*schema);
    db.GetEC ().GetSchemaManager ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (true, true));

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

    SetStruct2Values(struct2Instance1, L"string2-1", 1.001, struct1Instance1, struct1Instance2, struct1Instance3);
    SetStruct2Values(struct2Instance2, L"string2-2", 10.002, struct1Instance4, struct1Instance5, struct1Instance6);
    SetStruct2Values(struct2Instance3, L"string2-3", 15.003, struct1Instance7, struct1Instance8, struct1Instance9);

    ECN::StandaloneECInstancePtr testClassInstance = testClassEnabler->CreateInstance();
    testClassInstance->AddArrayElements(L"StructArray", 3);
    ECValue structVal1;
    structVal1.SetStruct(struct2Instance1.get());
    testClassInstance->SetValue(L"StructArray", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct(struct2Instance2.get());
    testClassInstance->SetValue(L"StructArray", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct(struct2Instance3.get());
    testClassInstance->SetValue(L"StructArray", structVal3, 2);

    //auto newInst = ECDbTestProject::CreateArbitraryECInstance (*testClass, PopulatePrimitiveValueWithRandomValues);
    auto persistence = db.GetEC().GetECPersistence (nullptr, *testClass);
    auto insertStatus = persistence->Insert (nullptr, *testClassInstance);
    ASSERT_EQ (INSERT_Success, insertStatus);
    //WString debugString = testClassInstance->ToString(L"");
    //wprintf(debugString.c_str());

    SqlPrintfString ecSql ("SELECT StructArray FROM TestSchema.TestClass");
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (db, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == status);

    while (ecStatement.Step() == ECSqlStepStatus::HasRow)
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
    //ECInstanceECSqlSelectAdapter dataAdapter (ecStatement, db);

    //IECInstancePtr resultInstance;
    //while (ecStatement.Step() == ECSqlStepStatus::HasRow)
    //    {
    //    resultInstance = dataAdapter.GetInstance();
    //    ASSERT_TRUE (resultInstance.IsValid());
    //    }

    //debugString = resultInstance->ToString(L"");
    //wprintf(debugString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances_Old, UpdateECInstances)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    ECDbR db = test.Create ("updateecinstances.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    auto const& ecDbStore = db.GetEC();

    // Check all the instances inserted
    ECInstanceMapCR importedInstances = test.GetImportedECInstances();
    for (ECInstanceMap::const_iterator iter = importedInstances.begin(); iter != importedInstances.end(); iter++)
        {
        ECInstanceId importedInstanceId = iter->first;
        IECInstancePtr importedInstance = iter->second;
        ECClassCR ecClass = importedInstance->GetClass();

        ECPersistencePtr persistence = ecDbStore.GetECPersistence (nullptr, ecClass);
        ASSERT_TRUE (persistence.IsValid());

        SqlPrintfString ecSql ("SELECT * FROM %s.%s WHERE ECInstanceId=%lld", Utf8String(ecClass.GetSchema().GetName()).c_str(), Utf8String(ecClass.GetName()).c_str(), importedInstanceId.GetValue());

        ECSqlStatement ecStatement;
        ECSqlStatus prepareStatus = ecStatement.Prepare (db, ecSql.GetUtf8CP());
        ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);

        ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);
        ECSqlStepStatus stepStatus = ecStatement.Step();
        ASSERT_TRUE(ECSqlStepStatus::HasRow == stepStatus);

        // First, verify that we can read what we wrote
        IECInstancePtr selectedInstance = dataAdapter.GetInstance();
        ASSERT_TRUE (selectedInstance.IsValid());
        bool areEqual = ECDbTestUtility::CompareECInstances (*importedInstance, *selectedInstance);
        if (!areEqual)
            {
            LOG.errorv(L"Updated ECInstance does not match expectations. Sending both to stdout. For %ls, id=%ls", 
                importedInstance->GetClass().GetFullName(), importedInstance->GetInstanceId().c_str());
            wprintf(importedInstance->ToString(L"IMPORT").c_str()); wprintf(L"\n");
            wprintf(selectedInstance->ToString(L"SELECT").c_str()); wprintf(L"\n");
            ASSERT_TRUE (false && "Selected ECInstance does not match expectations.");
            }

        // See if we can read it directly, too
        ecStatement.Reset();

        stepStatus = ecStatement.Step();
        ASSERT_TRUE(ECSqlStepStatus::HasRow == stepStatus);

        int nColumns = ecStatement.GetColumnCount();
        LOG.infov(L"Getting values for '%ls' (%lld) ECInstanceId=%lld", ecClass.GetFullName(), ecClass.GetId(), importedInstanceId.GetValue());
        for (int i = 0; i < nColumns; i++)
            {
            //EXPECT_TRUE (&ecClass == ecStatement.GetClass(i)); Not true when it is a property from an embedded struct.
            ECPropertyCP ecProperty = ecStatement.GetColumnInfo (i).GetProperty();
            ASSERT_TRUE (nullptr != ecProperty);

            WString propertyAccessString = ecProperty->GetName();//ecStatement.GetPropertyAccessString();

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
                    LOG.tracev("'%s'='%s'", Utf8String (propertyAccessString.c_str()).c_str (), dt.ToUtf8String ().c_str ());
                    }
                else if (primType == PRIMITIVETYPE_Point2D)
                    {
                    DPoint2d pt = ecStatement.GetValuePoint2D (i);
                    LOG.tracev("'%s'=(%.4f, %.4f)", Utf8String (propertyAccessString.c_str()).c_str (), pt.x, pt.y);
                    }
                else if (primType == PRIMITIVETYPE_Point3D)
                    {
                    DPoint3d pt = ecStatement.GetValuePoint3D (i);
                    LOG.tracev("'%s'=(%.4f, %.4f, %.4f)", Utf8String (propertyAccessString.c_str()).c_str (), pt.x, pt.y, pt.z);
                    }
                else
                    {
                    Utf8CP v = ecStatement.GetValueText (i);
                    int size = 0;
                    const byte* b = (const byte *) ecStatement.GetValueBinary(i, &size);
                    Utf8String valueAsString = v ? v : "";
                    LOG.tracev("'%s'='%s',  size=%d, b=0x%lx", Utf8String (propertyAccessString.c_str()).c_str (), valueAsString.c_str(), size, b);
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

        // update the instance with new data.
        UpdateStatus updateStatus = persistence->Update (*updateInst);
        //Empty classes are not updateable (an update would virtually be a no-op)
        //so the method returns an error and doesn't do anything.
        if (ecClass.GetPropertyCount (true) > 0)
            ASSERT_EQ (UPDATE_Success, updateStatus);
        else
            ASSERT_NE (UPDATE_Success, updateStatus);

        // now read back the instance from db
        ecStatement.Reset();
        stepStatus = ecStatement.Step();
        ASSERT_TRUE(ECSqlStepStatus::HasRow == stepStatus);

        IECInstancePtr actual = dataAdapter.GetInstance();
        ASSERT_TRUE (actual.IsValid());
        areEqual = ECDbTestUtility::CompareECInstances (*updateInst, *actual);
        if (!areEqual)
            {
            LOG.errorv(L"Updated ECInstance does not match expectations. Sending both to stdout. For %ls, id=%ls", 
                updateInst->GetClass().GetFullName(), updateInst->GetInstanceId().c_str());
            wprintf(updateInst->ToString(L"EXPECT").c_str()); wprintf(L"\n");
            wprintf(    actual->ToString(L"ACTUAL").c_str()); wprintf(L"\n");
            EXPECT_TRUE (false && "Updated ECInstance does not match expectations.");
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbInstances_Old, DeleteWithRelationshipBetweenStructs)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create("structs.ecdb");

    ECSchemaPtr structSchema;
    ECSchema::CreateSchema(structSchema, L"StructSchema", 1, 0);
    structSchema->SetNamespacePrefix(L"ss");
    structSchema->SetDescription(L"Schema with struct classes and a relationship between them");
    structSchema->SetDisplayLabel(L"Struct Schema");

    ECClassP struct1;
    structSchema->CreateClass (struct1, L"Struct1");
    ECClassP struct2;
    structSchema->CreateClass (struct2, L"Struct2");
    ECRelationshipClassP relationshipClass;
    structSchema->CreateRelationshipClass (relationshipClass, L"StructToStruct");

    struct1->SetIsStruct(true);
    struct1->SetIsDomainClass(true);
    struct2->SetIsStruct(true);
    struct2->SetIsDomainClass(true);
    
    ASSERT_TRUE(struct1->GetIsStruct());
    ASSERT_TRUE(struct1->GetIsDomainClass());

    ASSERT_TRUE(struct2->GetIsStruct());
    ASSERT_TRUE(struct2->GetIsDomainClass());

    PrimitiveECPropertyP stringProp1;
    struct1->CreatePrimitiveProperty (stringProp1, L"StringMember1");
    PrimitiveECPropertyP stringProp2;
    struct2->CreatePrimitiveProperty (stringProp2, L"StringMember2");

    relationshipClass->GetSource().AddClass(*struct1);
    relationshipClass->GetSource().SetCardinality(RelationshipCardinality::ZeroMany());
    relationshipClass->GetTarget().AddClass(*struct2);
    relationshipClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());

    // import
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*structSchema);

    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false)));

    auto struct1Inst = ECDbTestProject::CreateArbitraryECInstance(*struct1, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    PersistenceStatus persistStatus;
    ECPersistencePtr persistence1 = ecdb.GetEC().GetECPersistence (&persistStatus, *struct1);
    ASSERT_TRUE(persistence1.IsValid());
    ECInstanceId ecInstanceId1;
    auto insertStatus = persistence1->Insert (&ecInstanceId1, *struct1Inst);

    auto struct2Inst = ECDbTestProject::CreateArbitraryECInstance(*struct2, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
    ECPersistencePtr persistence2 = ecdb.GetEC().GetECPersistence (&persistStatus, *struct2);
    ASSERT_TRUE(persistence2.IsValid());
    ECInstanceId ecInstanceId2;
    insertStatus = persistence2->Insert (&ecInstanceId2, *struct2Inst);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relationshipClass);
    ASSERT_TRUE (relationshipEnabler.IsValid());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (struct1Inst.get());
    relationshipInstance->SetTarget (struct2Inst.get());
    relationshipInstance->SetInstanceId (L"source->target");
    
    ECPersistencePtr persistence3 = ecdb.GetEC().GetECPersistence (&persistStatus, *relationshipClass);
    ASSERT_TRUE(persistence3.IsValid());
    insertStatus = persistence3->Insert (nullptr, *relationshipInstance);

    DeleteStatus deleteStatus = persistence1->Delete (ecInstanceId1);
    ASSERT_EQ   (DELETE_Success, deleteStatus);

    }
    
END_ECDBUNITTESTS_NAMESPACE
