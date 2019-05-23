/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaConverterTests : ECTestFixture {};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    10/2018
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
// @bsimethod                                   Caleb.Shafer                    10/2018
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

END_BENTLEY_ECN_TEST_NAMESPACE
