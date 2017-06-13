/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECSchemaValidatorTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaValidatorTests : ECTestFixture {};
struct SchemaConverterTests : ECTestFixture {};

// Test valdiation and conversion of schema validation rules
Utf8CP oldStandardSchemaNames[] =
    {
    "Bentley_Standard_CustomAttributes",
    "Bentley_Standard_Classes",
    "Bentley_ECSchemaMap",
    "EditorCustomAttributes",
    "Bentley_Common_Classes",
    "Dimension_Schema",
    "iip_mdb_customAttributes",
    "KindOfQuantity_Schema",
    "rdl_customAttributes",
    "SIUnitSystemDefaults",
    "Unit_Attributes",
    "Units_Schema",
    "USCustomaryUnitSystemDefaults",
    "ECDbMap"
    };

Utf8CP newStandardSchemaNames[] =
    {
    "CoreClasses",
    "CoreCustomAttributes",
    "SchemaLocalizationCustomAttributes",
    };

void CheckStandardAsReference(ECSchemaPtr schema, Utf8CP schemaName, ECSchemaReadContextPtr context, bool shouldPassValidation, Utf8CP message)
    {
    SchemaKey refKey = SchemaKey(schemaName, 1, 0);
    ECSchemaPtr refSchema = context->LocateSchema(refKey, SchemaMatchType::Latest);
    ASSERT_TRUE(refSchema.IsValid());
    schema->AddReferencedSchema(*refSchema.get());
    EXPECT_TRUE(shouldPassValidation == ECSchemaValidator::Validate(*schema)) << message;
    schema->RemoveReferencedSchema(*refSchema);
    }

TEST_F(SchemaValidatorTests, TestLatestSchemaVersionValidation)
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
    // Test unsuccessfull validatation of previous XML version schema
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
    EXPECT_FALSE(ECSchemaValidator::Validate(*schema)) << "TestSchema failed to validate successfully even though it a valid EC3.1 schema due to its xml version not being the latest";
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
    EXPECT_TRUE(ECSchemaValidator::Validate(*schema)) << "TestSchema validates successfully as it is a valid EC3.1 schema";
    }
    // Test uncessful validation of previous version schema
    {
    Utf8CP badSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECEntityClass typeName="TestClass"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as the schema is not latest version";
    }
    }

TEST_F(SchemaValidatorTests, TestSchemaStandardReferences)
    {
    // Test uncessful validation of reference to standard schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
  //  for (Utf8CP* cur = oldStandardSchemaNames, *end = cur + _countof(oldStandardSchemaNames); cur < end; ++cur)
   //     CheckStandardAsReference(schema, *cur, context, false, "Old standard schemas are used as a reference. Validation should fail.");
    for (Utf8CP* cur = newStandardSchemaNames, *end = cur + _countof(newStandardSchemaNames); cur < end; ++cur)
        CheckStandardAsReference(schema, *cur, context, true, "New standard schemas are used as a reference. Validation should succeed.");

    // Use an updated ECDbMap schema as a reference
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="ECDbMap" alias="ts" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
    </ECSchema>)xml";
    ECSchemaPtr refSchema;
    ECSchema::ReadFromXmlString(refSchema, refXml, *context);
    ASSERT_TRUE(refSchema.IsValid());
    EXPECT_TRUE(refSchema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(ECSchemaValidator::Validate(*refSchema)) << "Should be a valid schema to later reference";

    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ref"/>
        <ECEntityClass typeName="TestClass"/>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as the referenced schema is the latest version of ECDbMap";
    }
    }

TEST_F(SchemaValidatorTests, MixinClassMayNotOverrideInheritedProperty)
    {
    // Test that a mixin class may not override an inherited property
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity0);
    mixin0->CreatePrimitiveProperty(prop, "P1");
    mixin1->AddBaseClass(*mixin0);

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Mixin property does not override anything so validation should succeed";
    mixin1->CreatePrimitiveProperty(prop, "P1");
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Mixin overrides an inherited property so validation should fail";
    }

TEST_F(SchemaValidatorTests, EntityClassMayNotInheritPropertyFromMultipleBaseClasses)
    {
    // Test that an entity class may not inherit a property from multiple base classes
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP baseEntity1;
    ECEntityClassP baseEntity2;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(baseEntity1, "BaseEntity1");
    schema->CreateEntityClass(baseEntity2, "BaseEntity2");
    entity0->AddBaseClass(*baseEntity1);

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Entity class and its base classes have no properties yet so validation should succeed";
    baseEntity1->CreatePrimitiveProperty(prop, "P1");
    entity0->CreatePrimitiveProperty(prop, "P1");
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Entity class may inherit a property from just one base class so validation should succeed";
    entity0->AddBaseClass(*baseEntity2);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Entity class may have multiple base classes so validation should succeed";
    baseEntity2->CreatePrimitiveProperty(prop, "P1");
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Entity class may not inherit a property from more than one base class so validation should fail";
    }
    
TEST_F(SchemaValidatorTests, EntityClassMayNotOverrideInheritedMixinProperty)
    {
    // Test that an entity class may not override a property inherited from mixin class
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP mixin0;
    PrimitiveECPropertyP prop;

    ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    mixin0->CreatePrimitiveProperty(prop, "P1");
    entity0->AddBaseClass(*mixin0);

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Entity class inherits a property from mixin class so validation should succeed";
    entity0->CreatePrimitiveProperty(prop, "P1");
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Entity class overrides a property inherited from mixin class so validation should fail";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
