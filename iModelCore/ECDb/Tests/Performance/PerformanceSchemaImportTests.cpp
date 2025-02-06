/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceSchemaImportTests : public ECDbTestFixture
    {
    static ECN::ECSchemaPtr CreateTestSchema(size_t noOfClass, size_t propertiesPerClass, bool customAttributeOnSchema, bool customAttributesOnClasses, bool customAttributesOnProperties, size_t NumberOfCustomAttributes);
    static void SetStruct1Val(StandaloneECInstancePtr instance, int intVal, Utf8CP stringVal, bool boolVal);
    static void SetStruct2Val(StandaloneECInstancePtr instance, Utf8CP stringVal, double doubleVal, StandaloneECInstancePtr struct1, StandaloneECInstancePtr struct2, StandaloneECInstancePtr struct3);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceSchemaImportTests::SetStruct1Val(StandaloneECInstancePtr instance, int intVal, Utf8CP stringVal, bool boolVal)
    {
    instance->SetValue("StructClassIntMember", ECValue(intVal));
    instance->SetValue("StructClassStringMember", ECValue(stringVal));
    instance->SetValue("StructClassBoolMember", ECValue(boolVal));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, SchemaWithCustomAttributeImportPerformance)
    {
    ECSchemaPtr ecSchema;
    for (int i = 0; i < 5; i++)
        {
        double importTime = 0.0;
        double schemaExportTime = 0.0;

        ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("schemaWithCAImportPerformance.ecdb"));

        ecSchema = PerformanceSchemaImportTests::CreateTestSchema(500, 10, true, true, true, i);
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
        testDescription.Sprintf("Schema with 500 Class, 10 properties each, with 1 CA on Schema, %d CA Per Class and %d CA per Property", i, i);
        LOGTODB(TEST_DETAILS, importTime, i, testDescription.c_str());
        testDescription.Sprintf("Schema with 500 Class, 10 properties each, with 1 CA on Schema, %d CA Per Class and %d CA per Property (ClearCache Time)", i, i);
        testDescription.Sprintf("Schema with 500 Class, 10 properties each, with 1 CA on Schema, %d CA Per Class and %d CA per Property (Schema Export Time)", i, i);
        LOGTODB(TEST_DETAILS, schemaExportTime, i, testDescription.c_str());
        m_ecdb.CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, ImportSimpleSchema)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ImportSimpleSchema.ecdb"));

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, DropSchemasPerformance)
    {
    auto schemaXml = Utf8String(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema_X" alias="ts_X" version="01.00.00" description="Test Schema_X" displayLabel="TestSchema_X" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestClass1_X" modifier="Sealed">
                <ECProperty propertyName="TestClassProperty1_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty2_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty3_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty4_X" typeName="int"/>
                <ECProperty propertyName="TestClassProperty5_X" typeName="string"/>
            </ECStructClass>
            <ECStructClass typeName="TestClass2_X" modifier="Sealed">
                <ECProperty propertyName="TestClassProperty6_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty7_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty8_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty9_X" typeName="int"/>
                <ECProperty propertyName="TestClassProperty10_X" typeName="string"/>
            </ECStructClass>
            <ECStructClass typeName="TestClass3_X" modifier="Sealed">
                <ECProperty propertyName="TestClassProperty11_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty12_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty13_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty14_X" typeName="int"/>
                <ECProperty propertyName="TestClassProperty15_X" typeName="string"/>
            </ECStructClass>
            <ECStructClass typeName="TestClass4_X" modifier="Sealed">
                <ECProperty propertyName="TestClassProperty16_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty17_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty18_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty19_X" typeName="int"/>
                <ECProperty propertyName="TestClassProperty20_X" typeName="string"/>
            </ECStructClass>
            <ECStructClass typeName="TestClass5_X" modifier="Sealed">
                <ECProperty propertyName="TestClassProperty21_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty22_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty23_X" typeName="string"/>
                <ECProperty propertyName="TestClassProperty24_X" typeName="int"/>
                <ECProperty propertyName="TestClassProperty25_X" typeName="string"/>
            </ECStructClass>

            <ECEntityClass typeName="TestClassArray1_X">
                <ECStructArrayProperty propertyName="TestPropArray1_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray2_X" typeName="TestClass2_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray3_X" typeName="TestClass3_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray2_X">
                <ECStructArrayProperty propertyName="TestPropArray4_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray5_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray6_X" typeName="TestClass5_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray3_X">
                <ECStructArrayProperty propertyName="TestPropArray7_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray8_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray4_X">
                <ECStructArrayProperty propertyName="TestPropArray9_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray10_X" typeName="TestClass2_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray11_X" typeName="TestClass3_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray12_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray13_X" typeName="TestClass5_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray5_X">
                <ECStructArrayProperty propertyName="TestPropArray14_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray15_X" typeName="TestClass2_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray16_X" typeName="TestClass3_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray17_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray18_X" typeName="TestClass5_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray6_X">
                <ECStructArrayProperty propertyName="TestPropArray19_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray20_X" typeName="TestClass2_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray21_X" typeName="TestClass3_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray22_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray23_X" typeName="TestClass5_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray7_X">
                <ECStructArrayProperty propertyName="TestPropArray24_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray25_X" typeName="TestClass2_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray26_X" typeName="TestClass3_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray27_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray28_X" typeName="TestClass5_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="TestClassArray8_X">
                <ECStructArrayProperty propertyName="TestPropArray29_X" typeName="TestClass1_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray30_X" typeName="TestClass2_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray31_X" typeName="TestClass3_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray32_X" typeName="TestClass4_X" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="TestPropArray33_X" typeName="TestClass5_X" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
        </ECSchema>
        )xml");

    const auto numSchemas = 100;
    T_Utf8StringVector schemaNames;
    for (auto i = 1; i <= numSchemas; ++i)
        schemaNames.push_back(Utf8PrintfString("TestSchema_%d", i));
    
    for (const auto& dropMultipleSchemas : { false /*Test DropSchema performance*/, true /*Test DropSchemas performance*/ })
        {
        // Setup ECDb and import schemas
        ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("DropSchemasPerformance.ecdb"));
        auto context = ECSchemaReadContext::CreateContext();
        ASSERT_TRUE(context.IsValid());
        context->AddSchemaLocater(m_ecdb.GetSchemaLocater());

        StopWatch timer(true);
        for (auto i = 1; i <= numSchemas; ++i)
            {
            auto tempSchemaXml = schemaXml;
            tempSchemaXml.ReplaceAll("_X", Utf8PrintfString("_%d", i).c_str());
            ECSchemaPtr schema;
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, tempSchemaXml.c_str(), *context));
            ASSERT_TRUE(schema.IsValid());
            }
        ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
        timer.Stop();

        LOGPERFDB(TEST_DETAILS, timer.GetElapsedSeconds(), "Importing schemas");

        for (const auto& schemaName : schemaNames)
            ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema(schemaName));

        timer.Start();
        if (dropMultipleSchemas)
            {
            ASSERT_TRUE(m_ecdb.Schemas().DropSchemas(schemaNames).IsSuccess());
            timer.Stop();
            LOGPERFDB(TEST_DETAILS, timer.GetElapsedSeconds(), "Dropping all imported schemas at once with DropSchemas function");
            }
        else
            {
            for (const auto& schemaName : schemaNames)
                ASSERT_TRUE(m_ecdb.Schemas().DropSchema(schemaName).IsSuccess());
            timer.Stop();
            LOGPERFDB(TEST_DETAILS, timer.GetElapsedSeconds(), "Dropping all imported schemas one at a time with DropSchema function");
            }
        for (const auto& schemaName : schemaNames)
            ASSERT_FALSE(m_ecdb.Schemas().ContainsSchema(schemaName));

        m_ecdb.AbandonChanges();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSchemaImportTests, CreateEmptyECDb)
    {
    const int opCount = 1000;
    StopWatch timer(true);
    for (int i = 0; i < opCount; i++)
        {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("empty.ecdb"));
        m_ecdb.CloseDb();
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, "Creating empty ECDb files");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, ImportProcessPhysicalSchema)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ImportProcessPhysicalSchema.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));

    ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    BeFileName schemasSearchPath;
    BeTest::GetHost().GetDocumentsRoot(schemasSearchPath);
    schemasSearchPath.AppendToPath(L"ECDb").AppendToPath(L"Schemas").AppendToPath(L"plant_10_2020");
    context->AddSchemaPath(schemasSearchPath);
    context->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    SchemaKey processPhysicalKey("ProcessPhysical", 1, 0, 1);
    {
    StopWatch timer(true);
    ECSchemaPtr processPhysical = context->LocateSchema(processPhysicalKey, SchemaMatchType::Latest);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "ProcessPhysical and References Loading from XML");
    }

    {
    StopWatch timer(true);
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    m_ecdb.CloseDb();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "ProcessPhysical and References Schema Import Test");
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, UpgradeLargeSchema_FlatHierarchy)
    {
    const int numberOfClasses = 150;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("UpgradeLargeSchema_FlatHierarchy.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="DwgAttributeDefinitions" alias="DwgAttdefs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract">
        </ECEntityClass>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract">
            <BaseClass>ElementAspect</BaseClass>
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)xml")));

    SchemaKey dwgSchemaKey ("DwgAttributeDefinitions", 1, 0, 0);
    ECSchemaPtr dwgSchema = context->GetFoundSchema (dwgSchemaKey, SchemaMatchType::LatestWriteCompatible);
    ECClassP elementMultiAspectClass = dwgSchema->GetClassP("ElementMultiAspect");
    EXPECT_NE (nullptr, elementMultiAspectClass);

    bvector<ECEntityClassP> classes;
    for(int nClass = 1; nClass <= numberOfClasses; nClass++)
        {
        ECEntityClassP newClass;
        Utf8PrintfString className("Class%d", nClass);
        EXPECT_EQ (ECN::ECObjectsStatus::Success, dwgSchema->CreateEntityClass(newClass, className.c_str()));
        newClass->AddBaseClass(*elementMultiAspectClass);
        classes.push_back(newClass);
        for(int nProp = 1; nProp <= nClass; nProp++) // create according to the number of the class. Class1 has 1 property, Class100 has 100 properties
            {
            PrimitiveECPropertyP property;
            Utf8PrintfString propertyName("Class%dProperty%d", nClass, nProp);
            EXPECT_EQ (ECN::ECObjectsStatus::Success, newClass->CreatePrimitiveProperty(property, propertyName, PrimitiveType::PRIMITIVETYPE_String));
            }
        }

    StopWatch timer(true);
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numberOfClasses, "Initial Schema Import.");

    dwgSchema->SetVersionWrite(2);
    for(auto c : classes)
        { //add 10 properties to each class
        for(int nProp = 1; nProp <= 10; nProp++)
            {
            PrimitiveECPropertyP property;
            Utf8PrintfString propertyName("%sAdditionalProperty%d", c->GetName().c_str(), nProp);
            EXPECT_EQ (ECN::ECObjectsStatus::Success, c->CreatePrimitiveProperty(property, propertyName, PrimitiveType::PRIMITIVETYPE_String));
            }
        }
    timer.Start();
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numberOfClasses, "Update Schema.");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, UpgradeLargeSchema_DeepHierarchy)
    {
    const int numberOfClasses = 50;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("UpgradeLargeSchema_DeepHierarchy.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="DwgAttributeDefinitions" alias="DwgAttdefs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract">
        </ECEntityClass>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract">
            <BaseClass>ElementAspect</BaseClass>
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)xml")));

    SchemaKey dwgSchemaKey ("DwgAttributeDefinitions", 1, 0, 0);
    ECSchemaPtr dwgSchema = context->GetFoundSchema (dwgSchemaKey, SchemaMatchType::LatestWriteCompatible);
    ECClassP elementMultiAspectClass = dwgSchema->GetClassP("ElementMultiAspect");
    EXPECT_NE (nullptr, elementMultiAspectClass);

    bvector<ECEntityClassP> classes;
    ECClassP previousClass = elementMultiAspectClass;
    for(int nClass = 1; nClass <= numberOfClasses; nClass++)
        {
        ECEntityClassP newClass;
        Utf8PrintfString className("Class%d", nClass);
        EXPECT_EQ (ECN::ECObjectsStatus::Success, dwgSchema->CreateEntityClass(newClass, className.c_str()));
        newClass->AddBaseClass(*previousClass);
        previousClass = newClass;
        classes.push_back(newClass);
        for(int nProp = 1; nProp <= 3; nProp++) // create according to the number of the class. Class1 has 1 property, Class100 has 100 properties
            {
            PrimitiveECPropertyP property;
            Utf8PrintfString propertyName("Class%dProperty%d", nClass, nProp);
            EXPECT_EQ (ECN::ECObjectsStatus::Success, newClass->CreatePrimitiveProperty(property, propertyName, PrimitiveType::PRIMITIVETYPE_String));
            }
        }

    StopWatch timer(true);
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numberOfClasses, "Initial Schema Import.");

    dwgSchema->SetVersionWrite(2);
    for(auto c : classes)
        { //add 10 properties to each class
        for(int nProp = 1; nProp <= 2; nProp++)
            {
            PrimitiveECPropertyP property;
            Utf8PrintfString propertyName("%sAdditionalProperty%d", c->GetName().c_str(), nProp);
            EXPECT_EQ (ECN::ECObjectsStatus::Success, c->CreatePrimitiveProperty(property, propertyName, PrimitiveType::PRIMITIVETYPE_String));
            }
        }
    timer.Start();
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numberOfClasses, "Update Schema.");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSchemaImportTests, UpgradeLargeSchema_ManyClassesAndProperties)
    {
    const int numberOfClasses = 1000;
    const int numberOfProperties = 70;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("UpgradeLargeSchema_ManyClassesAndProperties.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="DwgAttributeDefinitions" alias="DwgAttdefs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract">
        </ECEntityClass>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract">
            <BaseClass>ElementAspect</BaseClass>
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)xml")));

    SchemaKey dwgSchemaKey ("DwgAttributeDefinitions", 1, 0, 0);
    ECSchemaPtr dwgSchema = context->GetFoundSchema (dwgSchemaKey, SchemaMatchType::LatestWriteCompatible);
    ECClassP elementMultiAspectClass = dwgSchema->GetClassP("ElementMultiAspect");
    EXPECT_NE (nullptr, elementMultiAspectClass);

    for(int nClass = 1; nClass <= numberOfClasses; nClass++)
        {
        ECEntityClassP newClass;
        Utf8PrintfString className("Class%d", nClass);
        EXPECT_EQ (ECN::ECObjectsStatus::Success, dwgSchema->CreateEntityClass(newClass, className.c_str()));
        newClass->AddBaseClass(*elementMultiAspectClass);
        for(int nProp = 1; nProp <= numberOfProperties; nProp++) // create according to the number of the class. Class1 has 1 property, Class100 has 100 properties
            {
            PrimitiveECPropertyP property;
            Utf8PrintfString propertyName("Property%d", nProp);
            EXPECT_EQ (ECN::ECObjectsStatus::Success, newClass->CreatePrimitiveProperty(property, propertyName, PrimitiveType::PRIMITIVETYPE_String));
            }
        }

    StopWatch timer(true);
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numberOfClasses, "Initial Schema Import.");

    dwgSchema->SetVersionWrite(2);
    for(int nClass = 1; nClass <= numberOfClasses; nClass++)
        {
        ECEntityClassP newClass;
        Utf8PrintfString className("Class%d_2", nClass);
        EXPECT_EQ (ECN::ECObjectsStatus::Success, dwgSchema->CreateEntityClass(newClass, className.c_str()));
        newClass->AddBaseClass(*elementMultiAspectClass);
        for(int nProp = 1; nProp <= numberOfProperties; nProp++) // create according to the number of the class. Class1 has 1 property, Class100 has 100 properties
            {
            PrimitiveECPropertyP property;
            Utf8PrintfString propertyName("Property%d", nProp);
            EXPECT_EQ (ECN::ECObjectsStatus::Success, newClass->CreatePrimitiveProperty(property, propertyName, PrimitiveType::PRIMITIVETYPE_String));
            }
        }
    timer.Start();
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numberOfClasses, "Update Schema.");
    }
END_ECDBUNITTESTS_NAMESPACE