/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceSchemaImportTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UnitTests/NonPublished/ECDb/ECDbTestProject.h"
#include"PerformanceTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceTestsSchemaImport : public ::testing::Test
    {
    static ECN::ECSchemaPtr CreateTestSchema (size_t noOfClass, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes);
    static void SetStruct1Val (StandaloneECInstancePtr instance, int intVal, WCharCP stringVale, bool boolVal);
    static void SetStruct2Val (StandaloneECInstancePtr instance, WCharCP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3);
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestsSchemaImport::SetStruct1Val (StandaloneECInstancePtr instance, int intVal, WCharCP stringVale, bool boolVal)
    {
    instance->SetValue (L"StructClassIntMember", ECValue (intVal));
    instance->SetValue (L"StructClassStringMember", ECValue (stringVale));
    instance->SetValue (L"StructClassBoolMember", ECValue (boolVal));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestsSchemaImport::SetStruct2Val (StandaloneECInstancePtr instance, WCharCP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3)
    {
    instance->SetValue (L"Struct2StringMember", ECValue (stringVal));
    instance->SetValue (L"Struct2DoubleMember", ECValue (doubleVal));
    instance->AddArrayElements (L"NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct (struct1.get ());
    instance->SetValue (L"Struct2Array", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct (struct2.get ());
    instance->SetValue (L"Struct2Array", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct (struct3.get ());
    instance->SetValue (L"Struct2Array", structVal3, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr PerformanceTestsSchemaImport::CreateTestSchema (size_t noOfClasses, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes)
    {
    ECSchemaPtr testSchema;
    ECClassP testClass = nullptr;
    PrimitiveECPropertyP primitiveProperty = nullptr;

    if (NumberOfCustomAttributes > 4)
        NumberOfCustomAttributes = 4;

    ECSchema::CreateSchema (testSchema, L"TestSchema", 1, 0);
    EXPECT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix (L"ts");
    testSchema->SetDescription (L"Dynamic Test Schema");
    testSchema->SetDisplayLabel (L"Test Schema");

    auto readContext = ECSchemaReadContext::CreateContext ();
    auto bscaKey = SchemaKey (L"Bentley_Standard_CustomAttributes", 1, 11);
    auto bscaSchema = readContext->LocateSchema (bscaKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE (bscaSchema.IsValid ());
    EXPECT_EQ (testSchema->AddReferencedSchema (*bscaSchema), ECOBJECTS_STATUS_Success);

    //creating a nested Array CustomAttribute Class
    ECClassP struct1 = nullptr;
    testSchema->CreateClass (struct1, L"Struct1");
    EXPECT_TRUE (struct1->SetIsStruct (true) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (struct1->CreatePrimitiveProperty (primitiveProperty, L"StructClassIntMember", PrimitiveType::PRIMITIVETYPE_Integer) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (struct1->CreatePrimitiveProperty (primitiveProperty, L"StructClassStringMember", PrimitiveType::PRIMITIVETYPE_String) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (struct1->CreatePrimitiveProperty (primitiveProperty, L"StructClassBoolMember", PrimitiveType::PRIMITIVETYPE_String) == ECOBJECTS_STATUS_Success);

    ECClassP struct2 = nullptr;
    testSchema->CreateClass (struct2, L"struct2");
    EXPECT_TRUE (struct2->SetIsStruct (true) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (struct2->CreatePrimitiveProperty (primitiveProperty, L"Struct2StringMember", PrimitiveType::PRIMITIVETYPE_String) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (struct2->CreatePrimitiveProperty (primitiveProperty, L"Struct2DoubleMember", PrimitiveType::PRIMITIVETYPE_String) == ECOBJECTS_STATUS_Success);
    ArrayECPropertyP arrayProperty = nullptr;
    EXPECT_TRUE (struct2->CreateArrayProperty (arrayProperty, L"Struct2Array", struct1) == ECOBJECTS_STATUS_Success);


    ECClassP testClassCustomAttribute = nullptr;
    testSchema->CreateClass (testClassCustomAttribute, L"StructArrayCustomAttribute");
    EXPECT_TRUE (testClassCustomAttribute->SetIsCustomAttributeClass (true) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (testClassCustomAttribute->CreatePrimitiveProperty (primitiveProperty, L"intMember", PrimitiveType::PRIMITIVETYPE_Integer) == ECOBJECTS_STATUS_Success);
    ArrayECPropertyP nestedArrayProperty = nullptr;
    EXPECT_TRUE (testClassCustomAttribute->CreateArrayProperty (nestedArrayProperty, L"NestedArray", struct2) == ECOBJECTS_STATUS_Success);

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

    SetStruct1Val (struct1Instance1, 1, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance2, 2, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance3, 3, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance4, 4, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance5, 5, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance6, 6, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance7, 7, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val (struct1Instance8, 8, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val (struct1Instance9, 9, L"StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);

    ECN::StandaloneECInstancePtr struct2Instance1 = struct2Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct2Instance2 = struct2Enabler->CreateInstance ();
    ECN::StandaloneECInstancePtr struct2Instance3 = struct2Enabler->CreateInstance ();

    SetStruct2Val (struct2Instance1, L"StructArrayCustomAttribute_Struct2String1", 1.001, struct1Instance1, struct1Instance2, struct1Instance3);
    SetStruct2Val (struct2Instance2, L"StructArrayCustomAttribute_Struct2String2", 10.002, struct1Instance4, struct1Instance5, struct1Instance6);
    SetStruct2Val (struct2Instance3, L"StructArrayCustomAttribute_Struct2String3", 15.003, struct1Instance7, struct1Instance8, struct1Instance9);

    auto tc_CustomAttribute = testClassCustomAttribute->GetDefaultStandaloneEnabler ()->CreateInstance ();
    tc_CustomAttribute->AddArrayElements (L"NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct (struct2Instance1.get ());
    tc_CustomAttribute->SetValue (L"intMember", ECValue (1));
    tc_CustomAttribute->SetValue (L"NestedArray", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct (struct2Instance2.get ());
    tc_CustomAttribute->SetValue (L"NestedArray", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct (struct2Instance3.get ());
    tc_CustomAttribute->SetValue (L"NestedArray", structVal3, 2);

    if (customAttributeOnSchema)
        {
        auto ca = bscaSchema->GetClassCP (L"SchemaMap");
        EXPECT_TRUE (ca != nullptr);
        auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (customAttribute != nullptr);
        EXPECT_TRUE (customAttribute->SetValue (L"TablePrefix", ECValue ("t")) == ECOBJECTS_STATUS_Success);
        EXPECT_TRUE (testSchema->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
        }

    for (size_t j = 0; j < noOfClasses; j++)
        {
        WString class1;
        class1.Sprintf (L"Class%d", j);
        EXPECT_TRUE (testSchema->CreateClass (testClass, class1) == ECOBJECTS_STATUS_Success);

        if (customAttributesOnClasses)
            {
            for (size_t i = 1; i <= NumberOfCustomAttributes; i++)
                {
                switch (i)
                    {
                        case 1:
                            {
                            EXPECT_TRUE (testClass->SetCustomAttribute (*tc_CustomAttribute) == ECOBJECTS_STATUS_Success);
                            break;
                            }
                        case 2:
                            {
                            auto ca = bscaSchema->GetClassCP (L"AllowDuplicateLocalizedValues");
                            EXPECT_TRUE (ca != nullptr);
                            auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                            EXPECT_TRUE (customAttribute != nullptr);
                            EXPECT_TRUE (customAttribute->SetValue (L"Label", ECValue (true)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (customAttribute->SetValue (L"Description", ECValue (false)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
                            break;
                            }
                        case 3:
                            {
                            auto ca = bscaSchema->GetClassCP (L"DisplayOptions");
                            EXPECT_TRUE (ca != nullptr);
                            auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                            EXPECT_TRUE (customAttribute != nullptr);
                            EXPECT_TRUE (customAttribute->SetValue (L"Hidden", ECValue (true)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (customAttribute->SetValue (L"HideRelated", ECValue (false)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
                            break;
                            }
                        case 4:
                            {
                            auto ca = bscaSchema->GetClassCP (L"SearchOptions");
                            EXPECT_TRUE (ca != nullptr);
                            auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                            EXPECT_TRUE (customAttribute->SetValue (L"ShowWhenDerivedClassIsShown", ECValue (true)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (customAttribute->SetValue (L"SearchPolymorphically", ECValue (true)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (customAttribute->SetValue (L"Hidden", ECValue (false)) == ECOBJECTS_STATUS_Success);
                            EXPECT_TRUE (testClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
                            break;
                            }
                        default:
                            break;
                    }
                }
            }

        for (size_t i = 0; i < propertiesPerClass; i++)
            {
            WString temp;
            temp.Sprintf (L"_Property%d", i);
            WString prop = class1;
            prop.append (temp);
            EXPECT_TRUE (testClass->CreatePrimitiveProperty (primitiveProperty, prop, PrimitiveType::PRIMITIVETYPE_String) == ECOBJECTS_STATUS_Success);

            if (customAttributesOnProperties)
                {
                for (size_t i = 1; i <= NumberOfCustomAttributes; i++)
                    {
                    switch (i)
                        {
                            case 1:
                                {
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*tc_CustomAttribute) == ECOBJECTS_STATUS_Success);
                                break;
                                }
                            case 2:
                                {
                                auto ca = bscaSchema->GetClassCP (L"PropertyMap");
                                EXPECT_TRUE (ca != nullptr);
                                auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                                EXPECT_TRUE (customAttribute->SetValue (L"IsNullable", ECValue (true)) == ECOBJECTS_STATUS_Success);
                                EXPECT_TRUE (customAttribute->SetValue (L"IsUnique", ECValue (false)) == ECOBJECTS_STATUS_Success);
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
                                break;
                                }
                            case 3:
                                {
                                auto ca = bscaSchema->GetClassCP (L"DateTimeInfo");
                                EXPECT_TRUE (ca != nullptr);
                                auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                                EXPECT_TRUE (customAttribute->SetValue (L"DateTimeKind", ECValue ("Utc")) == ECOBJECTS_STATUS_Success);
                                EXPECT_TRUE (customAttribute->SetValue (L"DateTimeComponent", ECValue ("DateTime")) == ECOBJECTS_STATUS_Success);
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
                                break;
                                }
                            case 4:
                                {
                                auto ca = bscaSchema->GetClassCP (L"StrictComparisonOnly");
                                EXPECT_TRUE (ca != nullptr);
                                auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
                                EXPECT_TRUE (primitiveProperty->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);
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
TEST_F (PerformanceTestsSchemaImport, SchemaWithCustomAttributeImportPerformance)
    {
    ECDb ecdb;
    BeFileName applicationSchemaDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (applicationSchemaDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationSchemaDir);
    auto stat = ECDbTestUtility::CreateECDb (ecdb, nullptr, L"schemaImportPerformance.ecdb");
    ASSERT_EQ (stat, BE_SQLITE_OK);

    ECSchemaPtr ecSchema;
    ecSchema = PerformanceTestsSchemaImport::CreateTestSchema (10000, 100, false, false, false, 0);
    ASSERT_TRUE (ecSchema.IsValid ());
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ASSERT_EQ (SUCCESS, schemaCache->AddSchema (*ecSchema));

    StopWatch timer (true);
    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions ()));
    timer.Stop ();
    LOG.infov ("Schema Import took %.4f msecs.", timer.GetElapsedSeconds () * 1000.0);
    PerformanceTestingFrameWork performanceObjSchemaImport;
    EXPECT_TRUE(performanceObjSchemaImport.writeTodb(timer, "PerformanceTestsSchemaImport,SchemaWithCustomAttributeImportPerformance_Import", "Import EC Schema with 10,000 Custom Attributes"));

    ecdb.SaveChanges ();

    timer.Start ();
    ecdb.ClearECDbCache ();
    timer.Stop ();
    LOG.infov ("ClearCache took %.4f msecs.", timer.GetElapsedSeconds () * 1000.0);

    timer.Start ();
    ECSchemaCP ecschema = ecdb.Schemas ().GetECSchema ("TestSchema", true);
    timer.Stop ();
    LOG.infov ("Schema Export took %.4f msecs.", timer.GetElapsedSeconds () * 1000.0);
    EXPECT_TRUE(performanceObjSchemaImport.writeTodb(timer, "PerformanceTestsSchemaImport,SchemaWithCustomAttributeImportPerformance_GetSchema", "Gets the schema that has 10,000 Custom Attributes"));

    ASSERT_TRUE (ecschema != nullptr);

    ecdb.CloseDb ();
    }

END_ECDBUNITTESTS_NAMESPACE