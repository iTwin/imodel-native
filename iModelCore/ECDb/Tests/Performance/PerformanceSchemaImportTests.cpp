/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceSchemaImportTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceSchemaImportTests : public ::testing::Test
    {
    static ECN::ECSchemaPtr CreateTestSchema (size_t noOfClass, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes);
    static void SetStruct1Val (StandaloneECInstancePtr instance, int intVal, Utf8CP stringVal, bool boolVal);
    static void SetStruct2Val (StandaloneECInstancePtr instance, Utf8CP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceSchemaImportTests::SetStruct1Val (StandaloneECInstancePtr instance, int intVal, Utf8CP stringVal, bool boolVal)
    {
    instance->SetValue ("StructClassIntMember", ECValue (intVal));
    instance->SetValue ("StructClassStringMember", ECValue (stringVal));
    instance->SetValue ("StructClassBoolMember", ECValue (boolVal));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceSchemaImportTests::SetStruct2Val (StandaloneECInstancePtr instance, Utf8CP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3)
    {
    instance->SetValue ("Struct2StringMember", ECValue (stringVal));
    instance->SetValue ("Struct2DoubleMember", ECValue (doubleVal));
    instance->AddArrayElements ("NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct (struct1.get ());
    instance->SetValue ("Struct2Array", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct (struct2.get ());
    instance->SetValue ("Struct2Array", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct (struct3.get ());
    instance->SetValue ("Struct2Array", structVal3, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr PerformanceSchemaImportTests::CreateTestSchema (size_t noOfClasses, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes)
    {
    ECSchemaPtr testSchema;
    ECEntityClassP testClass = nullptr;
    PrimitiveECPropertyP primitiveProperty = nullptr;

    if (NumberOfCustomAttributes > 4)
        NumberOfCustomAttributes = 4;

    ECSchema::CreateSchema (testSchema, "TestSchema", 1, 0);
    EXPECT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix ("ts");
    testSchema->SetDescription ("Dynamic Test Schema");
    testSchema->SetDisplayLabel ("Test Schema");

    auto readContext = ECSchemaReadContext::CreateContext ();
    auto bscaKey = SchemaKey ("Bentley_Standard_CustomAttributes", 1, 11);
    auto bscaSchema = readContext->LocateSchema (bscaKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE (bscaSchema.IsValid ());
    EXPECT_EQ (testSchema->AddReferencedSchema (*bscaSchema), ECObjectsStatus::Success);

    //creating a nested Array CustomAttribute Class
    ECStructClassP struct1 = nullptr;
    testSchema->CreateStructClass (struct1, "Struct1");
    EXPECT_TRUE (struct1->CreatePrimitiveProperty (primitiveProperty, "StructClassIntMember", PrimitiveType::PRIMITIVETYPE_Integer) == ECObjectsStatus::Success);
    EXPECT_TRUE (struct1->CreatePrimitiveProperty (primitiveProperty, "StructClassStringMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);
    EXPECT_TRUE (struct1->CreatePrimitiveProperty (primitiveProperty, "StructClassBoolMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);

    ECStructClassP struct2 = nullptr;
    testSchema->CreateStructClass (struct2, "struct2");
    EXPECT_TRUE (struct2->CreatePrimitiveProperty (primitiveProperty, "Struct2StringMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);
    EXPECT_TRUE (struct2->CreatePrimitiveProperty (primitiveProperty, "Struct2DoubleMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);
    StructArrayECPropertyP arrayProperty = nullptr;
    EXPECT_TRUE (struct2->CreateStructArrayProperty (arrayProperty, "Struct2Array", struct1) == ECObjectsStatus::Success);


    ECCustomAttributeClassP testClassCustomAttribute = nullptr;
    testSchema->CreateCustomAttributeClass (testClassCustomAttribute, "StructArrayCustomAttribute");
    EXPECT_TRUE (testClassCustomAttribute->CreatePrimitiveProperty (primitiveProperty, "intMember", PrimitiveType::PRIMITIVETYPE_Integer) == ECObjectsStatus::Success);
    StructArrayECPropertyP nestedArrayProperty = nullptr;
    EXPECT_TRUE (testClassCustomAttribute->CreateStructArrayProperty (nestedArrayProperty, "NestedArray", struct2) == ECObjectsStatus::Success);

    StandaloneECEnablerPtr struct1Enabler = struct1->GetDefaultStandaloneEnabler ();
    StandaloneECEnablerPtr struct2Enabler = struct2->GetDefaultStandaloneEnabler ();

    ECN::StandaloneECInstancePtr struct1Instance1 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance2 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance3 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance4 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance5 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance6 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance7 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance8 = struct1Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct1Instance9 = struct1Enabler->CreateInstance ();

    SetStruct1Val (struct1Instance1, 1, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance2, 2, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance3, 3, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance4, 4, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance5, 5, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance6, 6, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance7, 7, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance8, 8, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance9, 9, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);

    ECN::StandaloneECInstancePtr struct2Instance1 = struct2Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct2Instance2 = struct2Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct2Instance3 = struct2Enabler->CreateInstance ();

    SetStruct2Val (struct2Instance1, "StructArrayCustomAttribute_Struct2String1", 1.001, struct1Instance1, struct1Instance2, struct1Instance3);
    SetStruct2Val (struct2Instance2, "StructArrayCustomAttribute_Struct2String2", 10.002, struct1Instance4, struct1Instance5, struct1Instance6);
    SetStruct2Val (struct2Instance3, "StructArrayCustomAttribute_Struct2String3", 15.003, struct1Instance7, struct1Instance8, struct1Instance9);

    auto tc_CustomAttribute = testClassCustomAttribute->GetDefaultStandaloneEnabler ()->CreateInstance ();
    tc_CustomAttribute->AddArrayElements ("NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct (struct2Instance1.get ());
    tc_CustomAttribute->SetValue ("intMember", ECValue (1));
    tc_CustomAttribute->SetValue ("NestedArray", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct (struct2Instance2.get ());
    tc_CustomAttribute->SetValue ("NestedArray", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct (struct2Instance3.get ());
    tc_CustomAttribute->SetValue ("NestedArray", structVal3, 2);

    if (customAttributeOnSchema)
        {
        auto ca = bscaSchema->GetClassCP ("PrimarySchemaMetaData");
        EXPECT_TRUE (ca != nullptr);
        auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (customAttribute != nullptr);
        EXPECT_TRUE (customAttribute->SetValue ("ContainsUnits", ECValue (false)) == ECObjectsStatus::Success);
        EXPECT_TRUE (testSchema->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
        }

    for (size_t j = 0; j < noOfClasses; j++)
        {
        Utf8String class1;
        class1.Sprintf ("Class%d", j);
        EXPECT_TRUE (testSchema->CreateEntityClass (testClass, class1) == ECObjectsStatus::Success);

        if (customAttributesOnClasses)
            {
            for (size_t i = 1; i <= NumberOfCustomAttributes; i++)
                {
                switch (i)
                    {
                        case 1:
                            {
                            EXPECT_TRUE (testClass->SetCustomAttribute (*tc_CustomAttribute) == ECObjectsStatus::Success);
                            break;
                            }
                        case 2:
                            {
                            auto ca = bscaSchema->GetClassCP ("AllowDuplicateLocalizedValues");
                            EXPECT_TRUE (ca != nullptr);
                            auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                            EXPECT_TRUE (customAttribute != nullptr);
                            EXPECT_TRUE (customAttribute->SetValue ("Label", ECValue (true)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (customAttribute->SetValue ("Description", ECValue (false)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
                            break;
                            }
                        case 3:
                            {
                            auto ca = bscaSchema->GetClassCP ("DisplayOptions");
                            EXPECT_TRUE (ca != nullptr);
                            auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                            EXPECT_TRUE (customAttribute != nullptr);
                            EXPECT_TRUE (customAttribute->SetValue ("Hidden", ECValue (true)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (customAttribute->SetValue ("HideRelated", ECValue (false)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
                            break;
                            }
                        case 4:
                            {
                            auto ca = bscaSchema->GetClassCP ("SearchOptions");
                            EXPECT_TRUE (ca != nullptr);
                            auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                            EXPECT_TRUE (customAttribute->SetValue ("ShowWhenDerivedClassIsShown", ECValue (true)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (customAttribute->SetValue ("SearchPolymorphically", ECValue (true)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (customAttribute->SetValue ("Hidden", ECValue (false)) == ECObjectsStatus::Success);
                            EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
                            break;
                            }
                        default:
                            break;
                    }
                }
            }

        for (size_t i = 0; i < propertiesPerClass; i++)
            {
            Utf8String temp;
            temp.Sprintf ("_Property%d", i);
            Utf8String prop = class1;
            prop.append (temp);
            EXPECT_TRUE (testClass->CreatePrimitiveProperty (primitiveProperty, prop, PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);

            if (customAttributesOnProperties)
                {
                for (size_t i = 1; i <= NumberOfCustomAttributes; i++)
                    {
                    switch (i)
                        {
                            case 1:
                                {
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*tc_CustomAttribute) == ECObjectsStatus::Success);
                                break;
                                }
                            case 2:
                                {
                                auto ca = bscaSchema->GetClassCP ("DisplayOptions");
                                EXPECT_TRUE (ca != nullptr);
                                auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                                EXPECT_TRUE (customAttribute != nullptr);
                                EXPECT_TRUE (customAttribute->SetValue ("Hidden", ECValue (true)) == ECObjectsStatus::Success);
                                EXPECT_TRUE (customAttribute->SetValue ("HideRelated", ECValue (false)) == ECObjectsStatus::Success);
                                EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
                                break;
                                }
                            case 3:
                                {
                                auto ca = bscaSchema->GetClassCP ("DateTimeInfo");
                                EXPECT_TRUE (ca != nullptr);
                                auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                                EXPECT_TRUE (customAttribute->SetValue ("DateTimeKind", ECValue ("Utc")) == ECObjectsStatus::Success);
                                EXPECT_TRUE (customAttribute->SetValue ("DateTimeComponent", ECValue ("DateTime")) == ECObjectsStatus::Success);
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
                                break;
                                }
                            case 4:
                                {
                                auto ca = bscaSchema->GetClassCP ("StrictComparisonOnly");
                                EXPECT_TRUE (ca != nullptr);
                                auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);
                                break;
                                }
                            default:
                                break;
                        }
                    }
                }
            }
        }
    return testSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PerformanceSchemaImportTests, SchemaWithCustomAttributeImportPerformance)
    {
    ECDb ecdb;
    BeFileName applicationSchemaDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (applicationSchemaDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationSchemaDir);


    ECSchemaPtr ecSchema;
    for (int i = 0; i < 5; i++)
        {
        double importTime = 0.0;
        double clearCacheTime = 0.0;
        double schemaExportTime = 0.0;

        auto stat = ECDbTestUtility::CreateECDb (ecdb, nullptr, L"schemaWithCAImportPerformance.ecdb");
        ASSERT_EQ (stat, BE_SQLITE_OK);
        ecSchema = PerformanceSchemaImportTests::CreateTestSchema (5000, 100, true, true, true, i);
        ASSERT_TRUE (ecSchema.IsValid ());
        ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
        ASSERT_EQ (ECObjectsStatus::Success, schemaCache->AddSchema (*ecSchema));

        StopWatch timer (true);
        ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions ()));
        timer.Stop ();
        importTime = timer.GetElapsedSeconds ();
        ecdb.SaveChanges ();

        timer.Start ();
        ecdb.ClearECDbCache ();
        timer.Stop ();
        clearCacheTime = timer.GetElapsedSeconds ();

        timer.Start ();
        ECSchemaCP ecschema = ecdb.Schemas ().GetECSchema ("TestSchema", true);
        timer.Stop ();
        schemaExportTime = timer.GetElapsedSeconds ();
        ASSERT_TRUE (ecschema != nullptr);

        Utf8String testDescription;
        testDescription.Sprintf ("Schema with 5000 Class - 100 properties each - with 1 CA on Schema - %d CA Per Class and %d CA per Property", i, i);
        LOGTODB (TEST_DETAILS, importTime, testDescription.c_str (), i);
        testDescription.Sprintf ("Schema with 5000 Class - 100 properties each - with 1 CA on Schema - %d CA Per Class and %d CA per Property (ClearCache Time)", i, i);
        LOGTODB (TEST_DETAILS, clearCacheTime, testDescription.c_str (), i);
        testDescription.Sprintf ("Schema with 5000 Class - 100 properties each - with 1 CA on Schema - %d CA Per Class and %d CA per Property (Schema Export Time)", i, i);
        LOGTODB (TEST_DETAILS, schemaExportTime, testDescription.c_str (), i);
        ecdb.CloseDb ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PerformanceSchemaImportTests, ImportSimpleSchema)
    {
    ECDbTestFixture::Initialize ();
    ECDb testecdb;
    ASSERT_TRUE(BE_SQLITE_OK == ECDbTestUtility::CreateECDb (testecdb, nullptr, L"ImportSimpleSchema.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk (schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE (schemaptr != NULL);
    schemaCache->AddSchema (*schemaptr);

    StopWatch timer (true);
    auto stat = testecdb.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false));
    timer.Stop ();
    ASSERT_EQ (SUCCESS, stat);

    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), "Simple Schema Import Test with NO CustomAttribute and Reference Schemas");
    }

END_ECDBUNITTESTS_NAMESPACE