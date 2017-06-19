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

Utf8CP badSchemaRoleLabelXml0 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="NoRoleLabelTestSchemaFail0" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
            <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "label0 - base refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "label0 - base is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>

           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" polymorphic = "true">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true">
                    <Class class="TestClass"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

Utf8CP badSchemaRoleLabelXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RoleLabelTestSchemaFail1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
            <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "label1 - base refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "label1 - base is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>

           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "label1 - derived refers to" polymorphic = "true">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true">
                    <Class class="TestClass"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

Utf8CP badSchemaRoleLabelXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RoleLabelTestSchemaFail2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
            <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "label2 - base refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "label2 - base is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>

           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" polymorphic = "true">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel = "label2 - derived is referenced by" polymorphic="true">
                    <Class class="TestClass"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

Utf8CP goodSchemaRoleLabelXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RoleLabelTestSchemaSucceed" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
            <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "goodLabel - base label source" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "goodLabel - base label target" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>

           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "goodLabel - derived label source" polymorphic = "true">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel = "goodLabel - derived label target" polymorphic="true" >
                    <Class class="TestClass"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

void CheckStandardAsReference(ECSchemaPtr schema, Utf8CP schemaName, ECSchemaReadContextPtr context, bool shouldPassValidation, Utf8CP message)
    {
    SchemaKey refKey = SchemaKey(schemaName, 1, 0);
    ECSchemaPtr refSchema = context->LocateSchema(refKey, SchemaMatchType::Latest);
    ASSERT_TRUE(refSchema.IsValid());
    schema->AddReferencedSchema(*refSchema.get());
    EXPECT_TRUE(shouldPassValidation == ECSchemaValidator::Validate(*schema)) << message;
    schema->RemoveReferencedSchema(*refSchema);
    }

void CheckRoleLabel(ECSchemaPtr schema, Utf8CP expectedSourceRoleLabel, Utf8CP expectedTargetRoleLabel, bool expectedValidSource, bool expectedValidTarget)
    {
    ASSERT_TRUE(schema.IsValid());

    ECClassCP ecClass = schema->GetClassCP("TestRelationship"); // Derived class
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
    ECRelationshipConstraintR source = relClass->GetSource();
    ECRelationshipConstraintR target = relClass->GetTarget();

    if (expectedValidSource && expectedValidTarget)
        {
        EXPECT_STREQ(expectedSourceRoleLabel, source.GetInvariantRoleLabel().c_str()) << "Expected a valid source and target, and the source is incorrect";
        EXPECT_STREQ(expectedTargetRoleLabel, target.GetInvariantRoleLabel().c_str()) << "Expected a valid source and target, and the target is incorrect";
        EXPECT_TRUE(source.IsRoleLabelDefinedLocally()) << "Expected source and target to be defined locally, but source is not";
        EXPECT_TRUE(target.IsRoleLabelDefinedLocally()) << "Expected source and target to be defined locally, but target is not";
        }
    else if (expectedValidSource)
        {
        EXPECT_STREQ(expectedSourceRoleLabel, source.GetInvariantRoleLabel().c_str()) << "Expected a valid source only, and the source is incorrect";
        EXPECT_STRNE(expectedTargetRoleLabel, target.GetInvariantRoleLabel().c_str()) << "Expected a valid source only. Target should not be correct";
        EXPECT_TRUE(source.IsRoleLabelDefinedLocally()) << "Expected source to be defined locally";
        EXPECT_FALSE(target.IsRoleLabelDefinedLocally()) << "Expected just the source to be defined locally, but the target is also defined";
        }
    else if (expectedValidTarget)
        {
        EXPECT_STREQ(expectedTargetRoleLabel, target.GetInvariantRoleLabel().c_str()) << "Expected a valid target only, and the source is incorrect";
        EXPECT_STRNE(expectedTargetRoleLabel, source.GetInvariantRoleLabel().c_str()) << "Expected a valid target only. Source should not be correct";
        EXPECT_TRUE(target.IsRoleLabelDefinedLocally()) << "Expected target to be defined locally";
        EXPECT_FALSE(source.IsRoleLabelDefinedLocally()) << "Expected just the target to be defined locally, but the source is also defined";
        }
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

TEST_F(SchemaValidatorTests, MixinClassMayOnlyHaveOneBaseClass)
    {
    // Test mixin base class validation
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    ECEntityClassP mixin2;
    ECEntityClassP mixin3;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity0);
    schema->CreateMixinClass(mixin2, "Mixin2", *entity0);
    schema->CreateMixinClass(mixin3, "Mixin3", *entity0);

    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Mixin has no base classes. Should succeed validation";
    mixin0->AddBaseClass(*mixin1);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Mixin has one base class. Should succeed validation";
    mixin0->AddBaseClass(*mixin2);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Mixin has two base classes. Should fail validation";
    mixin0->AddBaseClass(*mixin3);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Mixin has multiple base classes. Should fail validation";
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

TEST_F(SchemaValidatorTests, BisCoreAspectTests)
    {
    // Element Aspect Relationship Tests
    // Multi
    Utf8CP badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract" description="ElementMultiAspect Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="MultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementMultiAspect</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "There is no relationship, so validation should fail";
   
    Utf8CP badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="TestMultiAspect" modifier="Abstract" displayLabel="Test Element Multi-Aspect" description="An Element Multi-Aspect Test Description">
            <BaseClass>ElementMultiAspect</BaseClass>
        </ECEntityClass>
        
        <ECRelationshipClass typeName="ElementOwnsMultiAspects" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementMultiAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="TestRelationship" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="TestMultiAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema2;
    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema2, badSchemaXml2, *context2);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema2)) << "Missing base class in TestRelationship. Validation should fail.";

    Utf8CP badSchemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="TestMultiAspect" modifier="Abstract" displayLabel="Test Element Multi-Aspect" description="An Element Multi-Aspect Test Description">
            <BaseClass>ElementMultiAspect</BaseClass>
        </ECEntityClass>
        
        <ECRelationshipClass typeName="ElementOwnsMultiAspects" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementMultiAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="TestRelationship" strength="embedding" modifier="None">
            <BaseClass>ElementOwnsMultiAspects</BaseClass>
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="TestMultiAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema3;
    ECSchemaReadContextPtr context3 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema3, badSchemaXml3, *context3);
    ASSERT_TRUE(schema3.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema3)) << "Aspect relationship is polymorphic, so validation should fail.";

    Utf8CP goodSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="Schema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract" description="ElementMultiAspect Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="MultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementMultiAspect</BaseClass>
        </ECEntityClass>      
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "This not a bis schema, so validation should succeed as this rule does not apply";

    Utf8CP goodSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>

        <ECRelationshipClass typeName="ElementOwnsMultiAspects" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="ElementAspect"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementMultiAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema4;
    ECSchemaReadContextPtr context4 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema4, goodSchemaXml2, *context4);
    ASSERT_TRUE(schema4.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema4)) << "The relationship does not DERIVE from 'ElementOwnsMultiAspects', it IS 'ElementOwnsMultiAspects'. Validation should succeed.";

    Utf8CP goodSchemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementMultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="TestMultiAspect" modifier="Abstract" displayLabel="Test Element Multi-Aspect" description="An Element Multi-Aspect Test Description">
            <BaseClass>ElementMultiAspect</BaseClass>
        </ECEntityClass>
        
        <ECRelationshipClass typeName="ElementOwnsMultiAspects" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementMultiAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="TestRelationship" strength="embedding" modifier="None">
            <BaseClass>ElementOwnsMultiAspects</BaseClass>
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="false">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="false">
                <Class class="TestMultiAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema5;
    ECSchemaReadContextPtr context5 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema5, goodSchemaXml3, *context5);
    ASSERT_TRUE(schema5.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema5)) << "BisCore example of a valid multi aspect relationship. Validation should succeed";

    // Unique
    badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" description="ElementUniqueAspect Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="MultiAspect" modifier="Abstract" displayLabel="Element Unique-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementUniqueAspect</BaseClass>
        </ECEntityClass>      
    </ECSchema>)xml";
    ECSchemaPtr schema6;
    ECSchemaReadContextPtr context6 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema6, badSchemaXml1, *context6);
    ASSERT_TRUE(schema6.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema6)) << "There is no relationship, so validation should fail";

    badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="TestUniqueAspect" modifier="Abstract" displayLabel="Test Element Multi-Aspect" description="An Element Multi-Aspect Test Description">
            <BaseClass>ElementUniqueAspect</BaseClass>
        </ECEntityClass>
        
        <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementUniqueAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="TestRelationship" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="TestUniqueAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema7;
    ECSchemaReadContextPtr context7 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema7, goodSchemaXml2, *context7);
    ASSERT_TRUE(schema7.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema7)) << "Missing base class in TestRelationship. Validation should fail.";

    badSchemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="TestUniqueAspect" modifier="Abstract" displayLabel="Test Element Multi-Aspect" description="An Element Multi-Aspect Test Description">
            <BaseClass>ElementUniqueAspect</BaseClass>
        </ECEntityClass>
        
        <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementUniqueAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="TestRelationship" strength="embedding" modifier="None">
            <BaseClass>ElementOwnsUniqueAspect</BaseClass>
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="TestUniqueAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema8;
    ECSchemaReadContextPtr context8 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema8, badSchemaXml3, *context8);
    ASSERT_TRUE(schema8.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema8)) << "BisCore example of a valid unique aspect relationship. Validation should succeed";

    goodSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="Schema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" description="ElementUniqueAspect Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="MultiAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementUniqueAspect</BaseClass>
        </ECEntityClass>      
    </ECSchema>)xml";
    ECSchemaPtr schema9;
    ECSchemaReadContextPtr context9 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema9, goodSchemaXml1, *context9);
    ASSERT_TRUE(schema9.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema9)) << "This not a bis schema, so validation should succeed as this rule does not apply";

    goodSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" displayLabel="Element Unique-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>

        <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="ElementAspect"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementUniqueAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema10;
    ECSchemaReadContextPtr context10 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema10, goodSchemaXml2, *context10);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema10)) << "The relationship does not DERIVE from 'ElementUniqueAspect', it IS 'ElementUniqueAspect'. Validation should succeed.";

    goodSchemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Element" modifier="Abstract" description="Element Description"/>
        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="Element Aspect Description"/>
        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" displayLabel="Element Multi-Aspect" description="An Element Multi-Aspect Description">
            <BaseClass>ElementAspect</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="TestUniqueAspect" modifier="Abstract" displayLabel="Test Element Multi-Aspect" description="An Element Multi-Aspect Test Description">
            <BaseClass>ElementUniqueAspect</BaseClass>
        </ECEntityClass>
        
        <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementUniqueAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECRelationshipClass typeName="TestRelationship" strength="embedding" modifier="None">
            <BaseClass>ElementOwnsUniqueAspect</BaseClass>
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="false">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="false">
                <Class class="TestUniqueAspect"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema11;
    ECSchemaReadContextPtr context11 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema11, goodSchemaXml3, *context11);
    ASSERT_TRUE(schema11.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema11)) << "BisCore example of a valid unique aspect relationship. Validation should succeed";
    }

TEST_F(SchemaValidatorTests, EntityClassMayNotInheritFromCertainBisClasses)
    {
    // Class may not implement both bis:IParentElement and bis:ISubModeledElement
    Utf8CP bisSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="ISubModeledElement" modifier="Abstract" description="ISubModeledElement Description"/>
        <ECEntityClass typeName="IParentElement" modifier="Abstract" description="IParentElement Description"/>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, bisSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "BisCore succeeds validation";

    Utf8CP badBisElementXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BadSchemaThatUsesBis" alias="bis" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="1.0" alias="bis"/>
        <ECEntityClass typeName="BadClass" modifier="Abstract" description="BadClass Description">
            <BaseClass>bis:IParentElement</BaseClass>
            <BaseClass>bis:ISubModeledElement</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml";
    ECSchemaPtr schema2;
    ECSchema::ReadFromXmlString(schema2, badBisElementXml, *context);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema2)) << "Schema implements both IParentElement and ISubModeledElement so validation should fail.";

    Utf8CP goodBisElementXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="GoodSchemaThatUsesBis1" alias="bis" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="1.0" alias="bis"/>
        <ECEntityClass typeName="BadClass" modifier="Abstract" description="BadClass Description">
            <BaseClass>bis:IParentElement</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml";
    ECSchemaPtr schema3;
    ECSchema::ReadFromXmlString(schema3, goodBisElementXml1, *context);
    ASSERT_TRUE(schema3.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema3)) << "Schema implements only IParentElement so validation should succeed.";

    Utf8CP goodBisElementXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="GoodSchemaThatUsesBis2" alias="bis" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="1.0" alias="bis"/>
        <ECEntityClass typeName="BadClass" modifier="Abstract" description="BadClass Description">
            <BaseClass>bis:ISubModeledElement</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml";
    ECSchemaPtr schema4;
    ECSchema::ReadFromXmlString(schema4, goodBisElementXml2, *context);
    ASSERT_TRUE(schema4.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema4)) << "Schema implements only ISubModeledElement so validation should succeed.";
    }

TEST_F(SchemaValidatorTests, FindPropertiesWhichShouldBeNavigationProperties)
    {
    Utf8CP badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="TestClassBad">
        <ECProperty propertyName="PropNameId" typeName="long">
        </ECProperty>
    </ECEntityClass>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as the property name ends in 'Id' and the type is 'long'";

    Utf8CP badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="SourceClass"/>
    <ECEntityClass typeName="TargetClass"/>
    <ECRelationshipClass typeName="TestRelationshipBad" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="SourceClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="TargetClass"/>
        </Target>
        <ECProperty propertyName="PropNameId" typeName="long"/>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as the property name ends in 'Id' and the type is 'long'";


    Utf8CP goodSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="TestClassGood1">
        <ECProperty propertyName="PropName" typeName="long">
        </ECProperty>
    </ECEntityClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as the property name does not end in 'Id' even though the type is 'long'";

    Utf8CP goodSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="TestClassGood2">
        <ECProperty propertyName="PropNameId" typeName="double">
        </ECProperty>
    </ECEntityClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as the property name ends in 'Id' but is not type 'long'";

    Utf8CP goodSchemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="SourceClass"/>
    <ECEntityClass typeName="TargetClass"/>
    <ECRelationshipClass typeName="TestRelationshipGood" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="SourceClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="TargetClass"/>
        </Target>
        <ECProperty propertyName="PropNameId" typeName="double"/>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml3, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as the property name ends in 'Id' but is not type 'long'";

    Utf8CP goodSchemaXml4 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="SourceClass"/>
    <ECEntityClass typeName="TargetClass"/>
    <ECRelationshipClass typeName="TestRelationshipGood" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="SourceClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="TargetClass"/>
        </Target>
        <ECProperty propertyName="PropertyNameiD" typeName="long"/>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml4, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as the property name ends in 'iD' not 'Id'";
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

TEST_F(SchemaValidatorTests, RelationshipClassMustLocallyDefineRoleLabel)
    {
    // Relationship source and target role label must be defined locally

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaRoleLabelXml0, *context);
    CheckRoleLabel(schema, "label0 - derived refers to", "label0 - derived is referenced by", false, false);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Relationship target and source role label are not locally defined so validation should fail";
   
    ECSchema::ReadFromXmlString(schema, badSchemaRoleLabelXml1, *context);
    CheckRoleLabel(schema, "label1 - derived refers to", "label1 - derived is referenced by", true, false);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Relationship target role label is not locally defined so validation should fail";

    ECSchema::ReadFromXmlString(schema, badSchemaRoleLabelXml2, *context);
    CheckRoleLabel(schema, "label2 - derived refers to", "label2 - derived is referenced by", false, true);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Relationship source role label is not locally defined so validation should fail";
    
    ECSchema::ReadFromXmlString(schema, goodSchemaRoleLabelXml, *context);
    CheckRoleLabel(schema, "goodLabel - derived label source", "goodLabel - derived label target", true, true);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Relationship target and source role label is locally defined so validation should succeed";
    }

TEST_F(SchemaValidatorTests, RelationshipClassMustLocallyDefineConstraintClasses)
    {
    // Relationship source and target constraints must be defined locally

    Utf8CP badSchemaXml0 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ConstraintTestSchemaFail0" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass"/>

           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
               <BaseClass>Base</BaseClass>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml0, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "There is no source and target so validation of the constraint classes should fail";

    Utf8CP badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ConstraintTestSchemaFail1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass"/>

           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                    <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Constraint class is not defined locally in source so validation should fail";

    Utf8CP badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ConstraintTestSchemaFail2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass"/>

           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
        <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Constraint class is not defined locally in target so validation should fail";

    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ConstraintTestSchemaSucceed" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass"/>

           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Constraint class are defined locally in source and target so validation should succeed";
    }

TEST_F(SchemaValidatorTests, RelationshipClassMustLocallyDefineAbstractConstraints)
    {
    // Relationship source and target abstract constraints must be defined locally

    Utf8CP badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="AbstractTestSchemaFail1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="TestClass2">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="BaseClass"/>
          
           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Abstract constraint class is not defined locally in target so validation should fail";

    Utf8CP badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="AbstractTestSchemaFail2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="TestClass2">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="BaseClass"/>
          
           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true" >
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Abstract constraint class is not defined locally in source so validation should fail";

    Utf8CP badSchemaXml3 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="AbstractTestSchemaFail3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="TestClass2">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="BaseClass"/>
          
           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml3, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Abstract constraint class is not defined locally in source so validation should fail";

    Utf8CP badSchemaXml4 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="AbstractTestSchemaFail4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="TestClass2">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="BaseClass"/>
          
           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">    
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml4, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Abstract constraint class is not defined locally in target so validation should fail";
    }

TEST_F(SchemaValidatorTests, RelationshipClassConstraintMayNotBeAbstractIfOnlyOneConcreteConstraint)
    {
    Utf8CP badSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ConstraintTestSchemaFail" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass"/>

           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
           
        <ECRelationshipClass typeName="TestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true" abstractConstraint="TestClass">
                    <Class class = "TestClass"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true">
                    <Class class = "TestClass"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "There is an abstract constraint and only one constraint class in source and target so validation should fail";

    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="AbstractTestSchemaSucceed" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECEntityClass typeName="TestClass">
               <BaseClass>BaseClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="TestClass2">
               <BaseClass>TestClass</BaseClass>
           </ECEntityClass>
           <ECEntityClass typeName="BaseClass"/>
          
           <ECRelationshipClass typeName = "Base" strength = "referencing" modifier = "Abstract">
                <Source multiplicity = "(0..*)" roleLabel = "refers to" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "BaseClass"/>
                    <Class class = "TestClass2"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true" abstractConstraint="BaseClass">
                    <Class class = "BaseClass"/>
                    <Class class = "TestClass2"/>
                </Target>
           </ECRelationshipClass>
           
           <ECRelationshipClass typeName="GoodTestRelationship" description="Test description" displayLabel="Test label" modifier="None" strength="referencing">
                <BaseClass>Base</BaseClass>
                <Source multiplicity="(0..1)" roleLabel = "refers to" polymorphic = "true" abstractConstraint="TestClass">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Source>
                <Target multiplicity = "(0..*)" roleLabel = "is referenced by" polymorphic = "true" abstractConstraint="TestClass">
                    <Class class = "TestClass"/>
                    <Class class = "TestClass2"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml";
    ECSchemaPtr schema2;
    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema2, goodSchemaXml, *context2);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema2)) << "Abstract constraints are defined locally in source and target so validation should succeed";
    }

TEST_F(SchemaValidatorTests, RelationshipClassMayNotHaveHoldingStrength)
    {
    Utf8CP badSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="holding" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as strength attribute must not be set to 'holding'";
    
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should pass validation as strength attribute is set to 'embedding'";
    }

TEST_F(SchemaValidatorTests, RelationshipClassEmbeddingStrengthTests)
    {
    // Test forward direction
    // Source = 0..*
    Utf8CP badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as source multiplicity upper bound is greater than 1 while strength is embedding and forward";
    
    // Source = 1..*
    Utf8CP badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(1..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as source multiplicity upper bound is greater than 1 while strength is embedding and forward";
   
    // Source = 0..1
    Utf8CP goodSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as source multiplicity upper bound is not greater than 1 while strength is embedding and forward";

    // Source = 1..1
    Utf8CP goodSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as source multiplicity upper bound is not greater than 1 while strength is embedding and forward";

    // Test backward direction
    // Target = 0..*
    badSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="backward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as target multiplicity upper bound is greater than 1 while strength is embedding and backward";

    // Target = 1..*
    badSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="backward" modifier="Sealed">
        <Source multiplicity="(1..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(1..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as target multiplicity upper bound is greater than 1 while strength is embedding and backward";

    // Target = 0..1
    goodSchemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="backward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml1, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as target multiplicity upper bound is not greater than 1 while strength is embedding and backward";

    // Target = 1..1
    goodSchemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" strengthDirection="backward" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(1..1)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml2, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as target multiplicity upper bound is not greater than 1 while strength is embedding and backward";

    // No direction given, so forward is assumed
    // Source = 0..*
    Utf8CP badSchemaNoDirection = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, badSchemaNoDirection, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as direction is assumed to be forward with embedding strength, with multiplicity in source greater than 1";

    // Source = 0..1
    Utf8CP goodSchemaNoDirection = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="StandardSchemaReferenced" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="ExampleRelationship" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    ECSchema::ReadFromXmlString(schema, goodSchemaNoDirection, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Should succeed validation as direction is assumed to be forward, with multiplicity equal to 1";
    }

TEST_F(SchemaValidatorTests, EmbeddingRelationshipsShouldNotContainHasInClassName)
    {
    Utf8CP badSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BadSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="RelationshipHasBadString" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, badSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Should fail validation as relationship is embedding and contains 'Has'";
    
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="GoodSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="TestClass"/>
        <ECRelationshipClass typeName="RelationshipHasPotentiallyBadString" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="read from source to target" polymorphic="true">
            <Class class="TestClass"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="read from target to source" polymorphic="true">
            <Class class="TestClass"/>
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";
    ECSchemaPtr schema2;
    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema2, goodSchemaXml, *context2);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema2)) << "Should succeed validation as relationship is 'referncing', not 'embedding'";
    }

TEST_F(SchemaConverterTests, RelationshipClassMustLocallyDefineRoleLabelConversion)
    {
    // Relationship source and target role label must be defined locally. Conversion should fix this validation failure.
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    // Load bad schema 0
    ECSchema::ReadFromXmlString(schema, badSchemaRoleLabelXml0, *context);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Relationship target role label is not locally defined so validation should fail";
    CheckRoleLabel(schema, "label0 - derived refers to", "label0 - derived is referenced by", false, false);

    // Convert the schema
    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion should have succeeded";
    CheckRoleLabel(schema, "label0 - base refers to", "label0 - base is referenced by", true, true);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Schema is now converted so validation should succeed";

    // Load bad schema 1
    ECSchema::ReadFromXmlString(schema, badSchemaRoleLabelXml1, *context);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Relationship target role label is not locally defined so validation should fail";
    CheckRoleLabel(schema, "label1 - derived refers to", "label1 - derived is referenced by", true, false);

    // Convert the schema
    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion should have succeeded";
    CheckRoleLabel(schema, "label1 - derived refers to", "label1 - base is referenced by", true, true);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Schema is now converted so validation should succeed";
    
    // Load bad schema 2
    ECSchema::ReadFromXmlString(schema, badSchemaRoleLabelXml2, *context);
    CheckRoleLabel(schema, "label2 - derived refers to", "label2 - derived is referenced by", false, true);
    ASSERT_FALSE(ECSchemaValidator::Validate(*schema)) << "Relationship source role label is not locally defined so validation should fail";
    
    // Convert the schema
    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion should have succeeded";
    CheckRoleLabel(schema, "label2 - base refers to", "label2 - derived is referenced by", true, true);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Schema is now converted so validation should succeed";

    // Load good schema 1
    ECSchema::ReadFromXmlString(schema, goodSchemaRoleLabelXml, *context);
    CheckRoleLabel(schema, "goodLabel - derived label source", "goodLabel - derived label target", true, true);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Relationship target and source role label is locally defined so validation should succeed";

    // Convert the schema 
    EXPECT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Schema conversion should have succeeded";
    CheckRoleLabel(schema, "goodLabel - derived label source", "goodLabel - derived label target", true, true);
    ASSERT_TRUE(ECSchemaValidator::Validate(*schema)) << "Schema is now converted so validation should (still) succeed";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
