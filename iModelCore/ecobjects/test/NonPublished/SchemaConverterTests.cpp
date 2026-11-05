/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaConverterTests : ECTestFixture {};

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaConverterTests, RenameReservedWords)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestEntityClass" isDomainClass="true">
                <ECProperty propertyName="Id" typeName="string" />
                <ECProperty propertyName="ECInstanceId" typeName="string" />
                <ECProperty propertyName="ECClassId" typeName="string" />
                <ECProperty propertyName="SourceECInstanceId" typeName="string" />
                <ECProperty propertyName="SourceId" typeName="string" />
                <ECProperty propertyName="SourceECClassId" typeName="string" />
                <ECProperty propertyName="TargetECInstanceId" typeName="string" />
                <ECProperty propertyName="TargetId" typeName="string" />
                <ECProperty propertyName="TargetECClassId" typeName="string" />
            </ECClass>
            <ECClass typeName="TestStructClass" isStruct="true">
                <ECProperty propertyName="Id" typeName="string" />
                <ECProperty propertyName="ECInstanceId" typeName="string" />
                <ECProperty propertyName="ECClassId" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema, *context));
    
    ECClassCP entity = schema->GetClassCP("TestEntityClass");
    EXPECT_EQ(nullptr, entity->GetPropertyP("Id")) << "The Id property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, entity->GetPropertyP("ECClassId")) << "The ECClassId property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, entity->GetPropertyP("ECInstanceId")) << "The ECInstanceId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("TestSchema_Id_")) << "The Id property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("TestSchema_ECClassId_")) << "The ECClassId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("TestSchema_ECInstanceId_")) << "The ECInstanceId property is a reserved keyword and should have been renamed";

    EXPECT_NE(nullptr, entity->GetPropertyP("SourceECInstanceId")) << "The SourceECInstanceId property is allowed on Entity classes and should not be renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("SourceId")) << "The SourceId property is allowed on Entity classes and should not be renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("SourceECClassId")) << "The SourceECClassId property is allowed on Entity classes and should not be renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("TargetECInstanceId")) << "The TargetECInstanceId property is allowed on Entity classes and should not be renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("TargetId")) << "The TargetId property is allowed on Entity classes and should not be renamed";
    EXPECT_NE(nullptr, entity->GetPropertyP("TargetECClassId")) << "The TargetECClassId property is allowed on Entity classes and should not be renamed";

    ECClassCP structClass = schema->GetClassCP("TestStructClass");
    EXPECT_NE(nullptr, structClass->GetPropertyP("Id")) << "The Id property is not a reserved keyword for Struct classes and should not be renamed";
    EXPECT_NE(nullptr, structClass->GetPropertyP("ECClassId")) << "The ECClassId property is not a reserved keyword for Struct classes and should not be renamed";
    EXPECT_NE(nullptr, structClass->GetPropertyP("ECInstanceId")) << "The ECInstanceId property is not a reserved keyword for Struct classes and should not be renamed";
    EXPECT_EQ(nullptr, structClass->GetPropertyP("TestSchema_Id_")) << "The Id property is not a reserved keyword for Struct classes and should not be renamed";
    EXPECT_EQ(nullptr, structClass->GetPropertyP("TestSchema_ECClassId_")) << "The ECClassId property is not a reserved keyword for Struct classes and should not be renamed";
    EXPECT_EQ(nullptr, structClass->GetPropertyP("TestSchema_ECInstanceId_")) << "The ECInstanceId property is not a reserved keyword for Struct classes and should not be renamed";
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaConverterTests, RenameRelationshipReservedWords)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" isStruct="true"/>
            <ECClass typeName="B" isDomainClass="true"/>
            <ECRelationshipClass typeName="ARelB" isDomainClass="true" strength="referencing" strengthDirection="forward">
                <ECProperty propertyName="SourceECInstanceId" typeName="string" />
                <ECProperty propertyName="SourceId" typeName="string" />
                <ECProperty propertyName="SourceECClassId" typeName="string" />
                <ECProperty propertyName="TargetECInstanceId" typeName="string" />
                <ECProperty propertyName="TargetId" typeName="string" />
                <ECProperty propertyName="TargetECClassId" typeName="string" />
                <Source cardinality="(1,1)" polymorphic="true">
                    <Class class="A"/>
                </Source>
                <Target cardinality="(1,1)" polymorphic="true">
                    <Class class="B"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema, *context));

    ECClassCP relClass = schema->GetClassCP("ARelB");

    EXPECT_EQ(nullptr, relClass->GetPropertyP("SourceECInstanceId")) << "The SourceECInstanceId property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, relClass->GetPropertyP("SourceId")) << "The SourceId property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, relClass->GetPropertyP("SourceECClassId")) << "The SourceECClassId property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, relClass->GetPropertyP("TargetECInstanceId")) << "The TargetECInstanceId property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, relClass->GetPropertyP("TargetId")) << "The TargetId property is a reserved keyword and should have been renamed";
    EXPECT_EQ(nullptr, relClass->GetPropertyP("TargetECClassId")) << "The TargetECClassId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, relClass->GetPropertyP("TestSchema_SourceECInstanceId_")) << "The SourceECInstanceId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, relClass->GetPropertyP("TestSchema_SourceId_")) << "The SourceId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, relClass->GetPropertyP("TestSchema_SourceECClassId_")) << "The SourceECClassId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, relClass->GetPropertyP("TestSchema_TargetECInstanceId_")) << "The TargetECInstanceId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, relClass->GetPropertyP("TestSchema_TargetId_")) << "The TargetId property is a reserved keyword and should have been renamed";
    EXPECT_NE(nullptr, relClass->GetPropertyP("TestSchema_TargetECClassId_")) << "The TargetECClassId property is a reserved keyword and should have been renamed";
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(SchemaConverterTests, PruneEmptyNamedCategories)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="EditorCustomAttributes" version="1.03" prefix="beca"/>
            <ECClass typeName="Class1" isDomainClass="true">
                <ECProperty propertyName="propA" typeName="string">
                    <ECCustomAttributes>
                        <Category xmlns="EditorCustomAttributes.01.00">
                            <Name>ValidTestName</Name>
                            <DisplayLabel>test display label</DisplayLabel>
                            <Description>test description</Description>
                            <Priority>20200</Priority>
                        </Category>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="propB" typeName="string">
                    <ECCustomAttributes>
                        <Category xmlns="EditorCustomAttributes.01.00">
                            <Name></Name>
                            <DisplayLabel>test display label</DisplayLabel>
                            <Description>test description</Description>
                            <Priority>20200</Priority>
                        </Category>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema, *context));
    EXPECT_EQ(0, schema->GetReferencedSchemas().size()) << "Expect EditorCustomAttributes schema reference to be removed during conversion";

    ECClassCP class1 = schema->GetClassCP("Class1");

    EXPECT_NE(nullptr, class1->GetPropertyP("propA")->GetCategory()) << "propA's category had a valid name and it should therefore have a valid category reference";
    EXPECT_EQ(nullptr, class1->GetPropertyP("propB")->GetCategory()) << "propB's category had an invalid name and it should therefore have a null category";
    EXPECT_FALSE(class1->GetPropertyP("propB")->GetCustomAttribute("EditorCustomAttributes", "Category").IsValid()) << "propB's category had an invalid name so the Category CA should be removed";
    EXPECT_NE(nullptr, schema->GetPropertyCategoryCP("ValidTestName")) << "the first category has a valid name and should not have been pruned";
    EXPECT_EQ(nullptr, schema->GetPropertyCategoryCP("")) << "the second category has an invalid name and should have been pruned";
    }

TEST_F(SchemaConverterTests, EmptyStructPropertyRemoval)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="as" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="EmptyStruct"/>
            <ECEntityClass typeName="EmptyEntity">
                <ECStructProperty propertyName="EmptyStructProperty" typeName="EmptyStruct"/>
                <ECStructArrayProperty propertyName="EmptyStructArrProperty" typeName="EmptyStruct"/>
            </ECEntityClass>
            <ECStructClass typeName="FullStruct">
                <ECProperty propertyName="FullProp1" typeName="int"/>
                <ECProperty propertyName="FullProp2" typeName="int"/>
            </ECStructClass>
            <ECEntityClass typeName="FullEntity">
                <ECStructProperty propertyName="FullStructProperty" typeName="FullStruct"/>
                <ECStructArrayProperty propertyName="FullStructArrProperty" typeName="FullStruct"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ASSERT_EQ(2, schema->GetClassCP("EmptyEntity")->GetPropertyCount());
    ASSERT_EQ(2, schema->GetClassCP("FullEntity")->GetPropertyCount());

    ECSchemaConverter::Convert(*schema, *context);

    //showing after conversion, struct property with empty struct class is deleted, but normal struct properties are left alone
    ASSERT_EQ(0, schema->GetClassCP("EmptyEntity")->GetPropertyCount());
    ASSERT_EQ(2, schema->GetClassCP("FullEntity")->GetPropertyCount());
    }

TEST_F(SchemaConverterTests, GetContainerName)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 1, 1);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "testClass");
    PrimitiveECPropertyP ecProp;
    ecClass->CreatePrimitiveProperty(ecProp, "testProp", PRIMITIVETYPE_String);

    StandardCustomAttributeReferencesConverter converter;
    EXPECT_STREQ("ECSchema test", converter.GetContainerName(*schema).c_str());
    EXPECT_STREQ("ECClass testClass", converter.GetContainerName(*ecClass).c_str());
    EXPECT_STREQ("ECProperty testProp", converter.GetContainerName(*ecProp).c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaConverterTests, ValidateEC2ToEC3RoundTripWithAStructArrayproperty)
    {
    constexpr Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="08.11" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="StructTestClass" displayLabel="Struct Class Label" isStruct="True" isDomainClass="False">
                <ECProperty propertyName="X" typeName="string" displayLabel="X Vector" />
                <ECProperty propertyName="Y" typeName="string" displayLabel="Y Vector" />
                <ECProperty propertyName="Z" typeName="string" displayLabel="Z Vector" />
            </ECClass>
            <ECClass typeName="CATestClass" displayLabel="ClassLabel" isDomainClass="False" isCustomAttributeClass="True">
                <ECArrayProperty propertyName="PropName" typeName="StructTestClass" displayLabel="Prop Label" minOccurs="1" maxOccurs="unbounded" isStruct="True" />
            </ECClass>
        </ECSchema>)xml";

    auto verify = [](ECSchemaCR schema, Utf8CP stage)
        {
        const auto structClass = schema.GetClassCP("StructTestClass");
        ASSERT_NE(nullptr, structClass) << stage << ": StructTestClass missing";
        EXPECT_TRUE(structClass->IsStructClass()) << stage << ": StructTestClass should be an ECStructClass, got class type " << static_cast<int>(structClass->GetClassType());

        const auto caClass = schema.GetClassCP("CATestClass");
        ASSERT_NE(nullptr, caClass) << stage << ": CATestClass missing";
        EXPECT_TRUE(caClass->IsCustomAttributeClass()) << stage << ": CATestClass should be an ECCustomAttributeClass";

        const auto prop = caClass->GetPropertyP("PropName", false);
        ASSERT_NE(nullptr, prop) << stage << ": CATestClass.PropName missing";
        EXPECT_TRUE(prop->GetIsStructArray()) << stage << ": PropName should be a struct array, got typeName '" << prop->GetTypeName().c_str() << "'";

        const auto structArray = prop->GetAsStructArrayProperty();
        if (nullptr != structArray)
            {
            EXPECT_STREQ("StructTestClass", structArray->GetStructElementType().GetName().c_str()) << stage;
            EXPECT_EQ(1u, structArray->GetMinOccurs()) << stage << ": minOccurs should be preserved";
            }
        };

    // Step 1: Read the raw EC2.0 schema xml string
    const auto  context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->OriginalECXmlVersionLessThan(ECVersion::V3_0));
    verify(*schema, "after EC2 read");

    // Step 2: EC2 -> EC3 conversion
    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, *context)) << "Schema conversion failed";
    verify(*schema, "after ECSchemaConverter::Convert");
    EXPECT_TRUE(schema->OriginalECXmlVersionLessThan(ECVersion::V3_0));

    // Step 3: serialize as ECXml 3.1 and read back
    Utf8String xml31;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(xml31, ECVersion::V3_1));
    EXPECT_TRUE(xml31.ContainsI("<ECStructClass typeName=\"StructTestClass\"")) << xml31.c_str();
    EXPECT_TRUE(xml31.ContainsI("<ECStructArrayProperty propertyName=\"PropName\" typeName=\"StructTestClass\"")) << xml31.c_str();

    const auto context31 = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema31;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema31, xml31.c_str(), *context31));
    ASSERT_TRUE(schema31.IsValid());
    verify(*schema31, "after ECXml 3.1 round trip");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaConverterTests, EC2StructArrayOnCustomAttributeClass_ForceCustomAttributeClassDegradesToStringArray)
    {
    constexpr Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="08.11" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="StructTestClass" displayLabel="Struct Class Label" isStruct="True" isDomainClass="False">
                <ECProperty propertyName="X" typeName="string" displayLabel="X Vector" />
                <ECProperty propertyName="Y" typeName="string" displayLabel="Y Vector" />
                <ECProperty propertyName="Z" typeName="string" displayLabel="Z Vector" />
            </ECClass>
            <ECClass typeName="CATestClass" displayLabel="ClassLabel" isDomainClass="False" isCustomAttributeClass="True">
                <ECArrayProperty propertyName="PropName" typeName="StructTestClass" displayLabel="Prop Label" minOccurs="1" maxOccurs="unbounded" isStruct="True" />
            </ECClass>
        </ECSchema>)xml";

    // Conversion schema which forces the class "StructTestClass" to convert to a CA class
    constexpr Utf8CP conversionSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema_V3Conversion" nameSpacePrefix="conv" version="08.11" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECv3ConversionAttributes" version="01.00" prefix="V2ToV3" />
            
            <ECClass typeName="StructTestClass" isDomainClass="False">
                <ECCustomAttributes>
                    <ForceCustomAttributeClass xmlns="ECv3ConversionAttributes.01.00" />
                </ECCustomAttributes>
            </ECClass>
        </ECSchema>)xml";

    auto verify = [](ECSchemaCR schema, Utf8CP stage)
        {
        const auto structClass = schema.GetClassCP("StructTestClass");
        ASSERT_NE(nullptr, structClass) << stage << ": StructTestClass missing";
        EXPECT_TRUE(structClass->IsCustomAttributeClass()) << stage << ": StructTestClass should have been forced to an ECCustomAttributeClass, got class type " << static_cast<int>(structClass->GetClassType());

        const auto caClass = schema.GetClassCP("CATestClass");
        ASSERT_NE(nullptr, caClass) << stage << ": CATestClass missing";
        EXPECT_TRUE(caClass->IsCustomAttributeClass()) << stage;

        const auto prop = caClass->GetPropertyP("PropName", false);
        ASSERT_NE(nullptr, prop) << stage << ": CATestClass.PropName missing";
        EXPECT_FALSE(prop->GetIsStructArray()) << stage << ": PropName can no longer be a struct array because StructTestClass itself is not a struct anymore";
        EXPECT_TRUE(prop->GetIsPrimitiveArray()) << stage << ": PropName should have defaulted to a primitive array";

        const auto primArray = prop->GetAsPrimitiveArrayProperty();
        if (nullptr != primArray)
            {
            EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, primArray->GetPrimitiveElementType()) << stage << ": unresolvable typeName should default to string";
            EXPECT_STREQ("string", prop->GetTypeName().c_str()) << stage;
            }
        };

    // Step 1: Read the conversion schema and register it
    const auto conversionContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr conversionSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(conversionSchema, conversionSchemaXml, *conversionContext));
    ASSERT_TRUE(conversionSchema.IsValid());

    const auto context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(ECObjectsStatus::Success, context->AddConversionSchema(*conversionSchema));
    ASSERT_TRUE(context->LocateConversionSchemaFor("TestSchema", 8, 11).IsValid()) << "Conversion schema was not located; check its name and that its version is read-compatible with TestSchema 08.11";

    TestIssueListener issues;
    context->Issues().AddListener(issues);

    // Step 2: Read the schema with the conversion schema registered context
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    verify(*schema, "after EC2 read with ForceCustomAttributeClass");

    // The degradation must be surfaced through the read context's issue reporter
    EXPECT_EQ(issues.m_issues.size(), 1U);
    EXPECT_EQ(issues.m_issues[0].id, ECIssueId::EC_0065);
    EXPECT_STREQ(issues.m_issues[0].message.c_str(), "The array property 'TestSchema:CATestClass.PropName' has typeName 'StructTestClass' which resolves to ECClass 'TestSchema:StructTestClass' but that class is not an ECStructClass. The property will be read as a primitive array and its type will default to 'string'.");
    EXPECT_TRUE(schema->OriginalECXmlVersionLessThan(ECVersion::V3_0));

    // The conversion process should have converted the struct array to a string array with an appropriate warning logged.
    Utf8String xml2_0;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(xml2_0, ECVersion::V2_0));
    EXPECT_TRUE(xml2_0.ContainsI("<ECClass typeName=\"StructTestClass\"")) << xml2_0.c_str();
    EXPECT_TRUE(xml2_0.ContainsI("<ECArrayProperty propertyName=\"PropName\" typeName=\"string\"")) << xml2_0.c_str();
    EXPECT_FALSE(xml2_0.ContainsI("ECStructArrayProperty")) << xml2_0.c_str();

    // Step 3: Convertor will not undo the change
    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, *context)) << "Schema conversion failed";
    verify(*schema, "after ECSchemaConverter::Convert");

    // Step 4: Verify the persisted ECXml 3.1 schema
    Utf8String xml31;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(xml31, ECVersion::V3_1));
    EXPECT_TRUE(xml31.ContainsI("<ECCustomAttributeClass typeName=\"StructTestClass\"")) << xml31.c_str();
    EXPECT_TRUE(xml31.ContainsI("<ECArrayProperty propertyName=\"PropName\" typeName=\"string\"")) << xml31.c_str();
    EXPECT_FALSE(xml31.ContainsI("ECStructArrayProperty")) << xml31.c_str();

    // The ForceCustomAttributeClass CA lives only in the conversion schema and is never persisted
    EXPECT_FALSE(xml31.ContainsI("ForceCustomAttributeClass")) << xml31.c_str();

    const auto context31 = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema31;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema31, xml31.c_str(), *context31));
    ASSERT_TRUE(schema31.IsValid());
    verify(*schema31, "after ECXml 3.1 round trip");
    EXPECT_TRUE(schema->OriginalECXmlVersionLessThan(ECVersion::V3_0));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
