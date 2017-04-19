/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECSchemaValidatorTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaValidatorTests : ECTestFixture {};

TEST_F(SchemaValidatorTests, TestBaseECValidation)
    {
    // Test failure if not latest EC Version schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" isDomainClass="true"/>
            <ECRelationshipClass typeName="TestRelationship">
                <Source cardinality="(1,1)" polymorphic="true">
                    <Class class="TestClass"/>
                </Source>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_FALSE(ECSchemaValidator::Validate(*schema)) << "TestSchema validated successfully even though it is not a valid EC3.1 schema";
    }
    // Test successfully validates EC2.0 schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" isDomainClass="true"/>
            <ECClass typeName="A" isDomainClass="true"/>
            <ECRelationshipClass typeName="ARelB">
                <Source cardinality="(1,1)" polymorphic="true">
                    <Class class="TestClass"/>
                </Source>
                <Target cardinality="(1,1)" polymorphic="true">
                    <Class class="A"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_TRUE(ECSchemaValidator::Validate(*schema)) << "TestSchema failed to validate successfully even though it a valid EC3.1 schema";
    }
    // Test successfully validates EC3.1 schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
            <ECEntityClass typeName="A"/>
            <ECRelationshipClass typeName="ARelB" modifier="None">
                <Source multiplicity="(1..1)" polymorphic="true" roleLabel="Source">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(1..1)" polymorphic="true" roleLabel="Target">
                    <Class class="A"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_TRUE(ECSchemaValidator::Validate(*schema)) << "TestSchema failed to validate successfully even though it is a valid EC3.1 schema";
    }
    }

TEST_F(SchemaValidatorTests, MixinClassMayOnlyHaveOneBaseClass)
    {
    {
    // Invalid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    ECEntityClassP mixin2;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    entity1->AddBaseClass(*entity0);
    schema->CreateEntityClass(entity2, "Entity2");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity0);
    schema->CreateMixinClass(mixin2, "Mixin2", *entity0);
   
    mixin0->AddBaseClass(*mixin1);
    mixin0->AddBaseClass(*mixin2);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Mixin has more than one base class. Should fail validation";
    }
    {
    Utf8CP badSchema = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
            <ECEntityClass typeName="BaseClass1">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>TestClass</AppliesToEntityClass>   
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="BaseClass2">
            <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>TestClass</AppliesToEntityClass>   
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="TestClass"/>
            <ECEntityClass typeName="A">
                <BaseClass>BaseClass1</BaseClass>
                <BaseClass>BaseClass2</BaseClass>
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>TestClass</AppliesToEntityClass>   
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECRelationshipClass typeName="ARelB" modifier="None">
                <Source multiplicity="(1..1)" polymorphic="true" roleLabel="Source">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(1..1)" polymorphic="true" roleLabel="Target">
                    <Class class="A"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchema, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_FALSE(ECSchemaValidator::Validate(*schema)) << "Mixin has more than one base class. Should fail validation";
    }
    {
    // Valid mixin base class
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    entity1->AddBaseClass(*entity0);
    schema->CreateEntityClass(entity2, "Entity2");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity0);

    mixin0->AddBaseClass(*mixin1);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Mixin has one base class. Should succeed validation";
    }
    }

TEST_F(SchemaValidatorTests, MixinClassMayNotOverrideInheritedProperty)
    {
    {
    // Invalid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;

    ECEntityClassP mixin0;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    mixin0->CreatePrimitiveProperty(prop, "P1");
    prop->SetBaseProperty(prop);

    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Mixin overrides an inherited property so validation should fail";
    }
    {
    // Valid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;

    ECEntityClassP mixin0;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    mixin0->CreatePrimitiveProperty(prop, "P1");

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Mixin property does not override so validation should succeed";
    }
    }
TEST_F(SchemaValidatorTests, EntityClassMayNotInheritFromMultipleBaseClasses)
    {
    {
    // Invalid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP baseEntity1;
    ECEntityClassP baseEntity2;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(baseEntity1, "BaseEntity1");
    schema->CreateEntityClass(baseEntity2, "BaseEntity2");

    baseEntity1->CreatePrimitiveProperty(prop, "P1");
    baseEntity2->CreatePrimitiveProperty(prop, "P1");

    entity0->AddBaseClass(*baseEntity1);
    entity0->AddBaseClass(*baseEntity2);
    entity0->CreatePrimitiveProperty(prop, "P1");

    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Entity class may not inherit a property from more than one base class so validation should fail";
    }
    {
    // Valid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP baseEntity;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(baseEntity, "BaseEntity");

    baseEntity->CreatePrimitiveProperty(prop, "P1");
    entity0->AddBaseClass(*baseEntity);
    entity0->CreatePrimitiveProperty(prop, "P1");

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Entity class may  inherit a property from just one base class so validation should succeed";
    }
    }
TEST_F(SchemaValidatorTests, EntityClassMayNotInheritPropertyFromMixin)
    {
    {
    // Invalid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP mixin0;
    PrimitiveECPropertyP base;

    ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    entity0->CreatePrimitiveProperty(base, "P1");

    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    mixin0->CreatePrimitiveProperty(base, "P1");
    entity0->AddBaseClass(*mixin0);

    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Entity class overrides a property inherited from mixin class so validation should fail";
    }
    {
    // Valid schema test
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP mixin0;
    PrimitiveECPropertyP base;

    ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    entity0->CreatePrimitiveProperty(base, "P1");

    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    mixin0->CreatePrimitiveProperty(base, "P1");

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Mixin is not a base class so the property is not inherited from mixin class so validation should succeed";
    }
    }
END_BENTLEY_ECN_TEST_NAMESPACE