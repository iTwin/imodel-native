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

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
    
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

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));

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

    EXPECT_TRUE(ECSchemaConverter::Convert(*schema));
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
                <ECProperty propertyName="FullProp1" typeName="Type1"/>
                <ECProperty propertyName="FullProp2" typeName="Type2"/>
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

    ECSchemaConverter::Convert(*schema);

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

END_BENTLEY_ECN_TEST_NAMESPACE
