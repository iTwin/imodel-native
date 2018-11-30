/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceSchemaImportTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceSchemaImportTests : public ECDbTestFixture
    {
    static ECN::ECSchemaPtr CreateTestSchema(size_t noOfClass, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes);
    static void SetStruct1Val(StandaloneECInstancePtr instance, int intVal, Utf8CP stringVal, bool boolVal);
    static void SetStruct2Val(StandaloneECInstancePtr instance, Utf8CP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceSchemaImportTests::SetStruct1Val(StandaloneECInstancePtr instance, int intVal, Utf8CP stringVal, bool boolVal)
    {
    instance->SetValue("StructClassIntMember", ECValue(intVal));
    instance->SetValue("StructClassStringMember", ECValue(stringVal));
    instance->SetValue("StructClassBoolMember", ECValue(boolVal));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceSchemaImportTests::SetStruct2Val(StandaloneECInstancePtr instance, Utf8CP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3)
    {
    instance->SetValue("Struct2StringMember", ECValue(stringVal));
    instance->SetValue("Struct2DoubleMember", ECValue(doubleVal));
    instance->AddArrayElements("NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct(struct1.get());
    instance->SetValue("Struct2Array", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct(struct2.get());
    instance->SetValue("Struct2Array", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct(struct3.get());
    instance->SetValue("Struct2Array", structVal3, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr PerformanceSchemaImportTests::CreateTestSchema(size_t noOfClasses, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes)
    {
    ECSchemaPtr testSchema;
    ECEntityClassP testClass = nullptr;
    PrimitiveECPropertyP primitiveProperty = nullptr;

    if (NumberOfCustomAttributes > 4)
        NumberOfCustomAttributes = 4;

    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 0);
    EXPECT_TRUE(testSchema.IsValid());
    testSchema->SetDescription("Dynamic Test Schema");
    testSchema->SetDisplayLabel("Test Schema");

    ECCustomAttributeClassP testSchemaCustomAttribute = nullptr;
    testSchema->CreateCustomAttributeClass(testSchemaCustomAttribute, "SchemaLevelCustomAttribute");

    if (customAttributeOnSchema)
        {
        testSchema->SetCustomAttribute(*testSchemaCustomAttribute->GetDefaultStandaloneEnabler()->CreateInstance());
        }

    //creating a nested Array CustomAttribute Class
    ECStructClassP struct1 = nullptr;
    testSchema->CreateStructClass(struct1, "Struct1");
    EXPECT_TRUE(struct1->CreatePrimitiveProperty(primitiveProperty, "StructClassIntMember", PrimitiveType::PRIMITIVETYPE_Integer) == ECObjectsStatus::Success);
    EXPECT_TRUE(struct1->CreatePrimitiveProperty(primitiveProperty, "StructClassStringMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);
    EXPECT_TRUE(struct1->CreatePrimitiveProperty(primitiveProperty, "StructClassBoolMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);

    ECStructClassP struct2 = nullptr;
    testSchema->CreateStructClass(struct2, "struct2");
    EXPECT_TRUE(struct2->CreatePrimitiveProperty(primitiveProperty, "Struct2StringMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);
    EXPECT_TRUE(struct2->CreatePrimitiveProperty(primitiveProperty, "Struct2DoubleMember", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);
    StructArrayECPropertyP arrayProperty = nullptr;
    EXPECT_TRUE(struct2->CreateStructArrayProperty(arrayProperty, "Struct2Array", *struct1) == ECObjectsStatus::Success);


    ECCustomAttributeClassP testClassCustomAttribute = nullptr;
    testSchema->CreateCustomAttributeClass(testClassCustomAttribute, "StructArrayCustomAttribute");
    EXPECT_TRUE(testClassCustomAttribute->CreatePrimitiveProperty(primitiveProperty, "intMember", PrimitiveType::PRIMITIVETYPE_Integer) == ECObjectsStatus::Success);
    StructArrayECPropertyP nestedArrayProperty = nullptr;
    EXPECT_TRUE(testClassCustomAttribute->CreateStructArrayProperty(nestedArrayProperty, "NestedArray", *struct2) == ECObjectsStatus::Success);

    StandaloneECEnablerPtr struct1Enabler = struct1->GetDefaultStandaloneEnabler();
    StandaloneECEnablerPtr struct2Enabler = struct2->GetDefaultStandaloneEnabler();

    ECN::StandaloneECInstancePtr struct1Instance1 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance2 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance3 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance4 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance5 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance6 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance7 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance8 = struct1Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct1Instance9 = struct1Enabler->CreateInstance();

    SetStruct1Val(struct1Instance1, 1, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val(struct1Instance2, 2, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val(struct1Instance3, 3, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val(struct1Instance4, 4, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val(struct1Instance5, 5, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val(struct1Instance6, 6, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val(struct1Instance7, 7, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);
    SetStruct1Val(struct1Instance8, 8, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", true);
    SetStruct1Val(struct1Instance9, 9, "StructArrayCustomAttribute_Struct2String1_struct1Instance1", false);

    ECN::StandaloneECInstancePtr struct2Instance1 = struct2Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct2Instance2 = struct2Enabler->CreateInstance();
    ECN::StandaloneECInstancePtr struct2Instance3 = struct2Enabler->CreateInstance();

    SetStruct2Val(struct2Instance1, "StructArrayCustomAttribute_Struct2String1", 1.001, struct1Instance1, struct1Instance2, struct1Instance3);
    SetStruct2Val(struct2Instance2, "StructArrayCustomAttribute_Struct2String2", 10.002, struct1Instance4, struct1Instance5, struct1Instance6);
    SetStruct2Val(struct2Instance3, "StructArrayCustomAttribute_Struct2String3", 15.003, struct1Instance7, struct1Instance8, struct1Instance9);

    auto tc_CustomAttribute = testClassCustomAttribute->GetDefaultStandaloneEnabler()->CreateInstance();
    tc_CustomAttribute->AddArrayElements("NestedArray", 3);
    ECValue structVal1;
    structVal1.SetStruct(struct2Instance1.get());
    tc_CustomAttribute->SetValue("intMember", ECValue(1));
    tc_CustomAttribute->SetValue("NestedArray", structVal1, 0);

    ECValue structVal2;
    structVal2.SetStruct(struct2Instance2.get());
    tc_CustomAttribute->SetValue("NestedArray", structVal2, 1);

    ECValue structVal3;
    structVal3.SetStruct(struct2Instance3.get());
    tc_CustomAttribute->SetValue("NestedArray", structVal3, 2);

    for (size_t j = 0; j < noOfClasses; j++)
        {
        Utf8String class1;
        class1.Sprintf("Class%d", j);
        EXPECT_TRUE(testSchema->CreateEntityClass(testClass, class1) == ECObjectsStatus::Success);

        if (customAttributesOnClasses)
            {
            for (size_t i = 1; i <= NumberOfCustomAttributes; i++)
                {
                EXPECT_TRUE(testClass->SetCustomAttribute(*tc_CustomAttribute) == ECObjectsStatus::Success);
                }
            }

        for (size_t i = 0; i < propertiesPerClass; i++)
            {
            Utf8String temp;
            temp.Sprintf("_Property%d", i);
            Utf8String prop = class1;
            prop.append(temp);
            EXPECT_TRUE(testClass->CreatePrimitiveProperty(primitiveProperty, prop, PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);

            if (customAttributesOnProperties)
                {
                for (size_t i = 1; i <= NumberOfCustomAttributes; i++)
                    {
                    EXPECT_TRUE(primitiveProperty->SetCustomAttribute(*tc_CustomAttribute) == ECObjectsStatus::Success);
                    }
                }
            }
        }
    return testSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, SchemaWithCustomAttributeImportPerformance)
    {
    ECSchemaPtr ecSchema;
    for (int i = 0; i < 5; i++)
        {
        double importTime = 0.0;
        double schemaExportTime = 0.0;

        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemaWithCAImportPerformance.ecdb"));

        ecSchema = PerformanceSchemaImportTests::CreateTestSchema(5000, 100, true, true, true, i);
        ASSERT_TRUE(ecSchema.IsValid());
        bvector<ECSchemaCP> schemas;
        schemas.push_back(ecSchema.get());

        StopWatch timer(true);
        ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(schemas));
        timer.Stop();
        importTime = timer.GetElapsedSeconds();
        m_ecdb.SaveChanges();

        timer.Start();
        ECSchemaCP ecschema = m_ecdb.Schemas().GetSchema("TestSchema", true);
        timer.Stop();
        schemaExportTime = timer.GetElapsedSeconds();
        ASSERT_TRUE(ecschema != nullptr);

        Utf8String testDescription;
        testDescription.Sprintf("Schema with 5000 Class, 100 properties each, with 1 CA on Schema, %d CA Per Class and %d CA per Property", i, i);
        LOGTODB(TEST_DETAILS, importTime, i, testDescription.c_str());
        testDescription.Sprintf("Schema with 5000 Class, 100 properties each, with 1 CA on Schema, %d CA Per Class and %d CA per Property (ClearCache Time)", i, i);
        testDescription.Sprintf("Schema with 5000 Class, 100 properties each, with 1 CA on Schema, %d CA Per Class and %d CA per Property (Schema Export Time)", i, i);
        LOGTODB(TEST_DETAILS, schemaExportTime, i, testDescription.c_str());
        m_ecdb.CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, ImportSimpleSchema)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ImportSimpleSchema.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Simple" alias="s" version="01.00" displayLabel="Basic Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo" modifier="None"/>
        </ECSchema>)xml")));

    StopWatch timer(true);
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "Simple Schema Import Test with just a single class w/o props");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSchemaImportTests, CreateEmptyECDb)
    {
    const int opCount = 1000;
    StopWatch timer(true);
    for (int i = 0; i < opCount; i++)
        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("empty.ecdb"));
        m_ecdb.CloseDb();
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, "Creating empty ECDb files");
    }


END_ECDBUNITTESTS_NAMESPACE