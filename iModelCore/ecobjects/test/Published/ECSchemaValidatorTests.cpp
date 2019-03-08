/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECSchemaValidatorTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

static Utf8CP bisSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="bis" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="1.0" alias="CoreCA"/>

        <ECEntityClass typeName="IParentElement" modifier="Abstract" description="IParentElement Description">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.1.0">
                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>

        <ECEntityClass typeName="Element" modifier="Abstract" description="Element description"/>
        <ECEntityClass typeName="PhysicalElement" modifier="Abstract" description="Element description">
            <BaseClass>Element</BaseClass>
        </ECEntityClass>

        <ECEntityClass typeName="ElementAspect" modifier="Abstract" displayLabel="Element Aspect" description="An Element Aspect is a class that defines a set of properties that are related to (and owned by) a single element. Semantically, an Element Aspect can be considered part of the Element. Thus, an Element Aspect is deleted if its owning Element is deleted."/>

        <ECEntityClass typeName="ElementUniqueAspect" modifier="Abstract" displayLabel="Element Unique Aspect" description="An Element Unique Aspect is an Element Aspect where there can be only zero or one instance of the Element Aspect class per Element.">
            <BaseClass>ElementAspect</BaseClass>
            <ECNavigationProperty propertyName="Element" relationshipName="ElementOwnsUniqueAspect" direction="Backward" description="The owning Element">
                <ECCustomAttributes>
                    <HiddenProperty xmlns="CoreCustomAttributes.1.0"/>
                </ECCustomAttributes>
            </ECNavigationProperty>
        </ECEntityClass>

        <ECRelationshipClass typeName="ElementOwnsUniqueAspect" strength="embedding" modifier="None">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
                <Class class="ElementUniqueAspect"/>
            </Target>
        </ECRelationshipClass>

        <ECEntityClass typeName="Model" modifier="Abstract" description="A Model is a container for persisting a collection of related elements.">
            <ECProperty propertyName="IsPrivate" typeName="boolean" displayLabel="Is Private" description="If IsPrivate is true then this model should not appear in lists shown to the user.">
                <ECCustomAttributes>
                    <HiddenProperty xmlns="CoreCustomAttributes.1.0"/>
                </ECCustomAttributes>
            </ECProperty>
            <ECProperty propertyName="IsTemplate" typeName="boolean" displayLabel="Is Template" description="IsTemplate will be true if this Model is used as a template for creating new instances.">
                <ECCustomAttributes>
                    <HiddenProperty xmlns="CoreCustomAttributes.1.0"/>
                </ECCustomAttributes>
            </ECProperty>
            <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json" displayLabel="JSON Properties" description="A string property that users and/or applications can use to persist JSON values.">
                <ECCustomAttributes>
                    <HiddenProperty xmlns="CoreCustomAttributes.1.0"/>
                </ECCustomAttributes>
            </ECProperty>
        </ECEntityClass>
        <ECEntityClass typeName="PhysicalModel" modifier="Abstract">
            <BaseClass>Model</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="DefinitionModel" modifier="Abstract">
            <BaseClass>Model</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="SpatialLocationModel" modifier="Abstract">
            <BaseClass>Model</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="SpatialModel" modifier="Abstract">
            <BaseClass>Model</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml";

struct SchemaValidatorTests : ECTestFixture
{
private:
    ECSchemaPtr m_testBis;

public:
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context;
    ECSchemaValidator validator;

    void InitContextWithSchemaXml(Utf8CP schemaXml)
        {
        context = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(schema, schemaXml, *context);
        }
    void InitBisContextWithSchemaXml(Utf8CP schemaXml)
        {
        context = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(m_testBis, bisSchemaXml, *context);
        ECSchema::ReadFromXmlString(schema, schemaXml, *context);
        }
};

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
    "CoreCustomAttributes",
    "SchemaLocalizationCustomAttributes",
    };

//---------------------------------------------------------------------------------------
// @bsimethod                             Dan.Perlman                          05/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, TestLatestSchemaVersionValidation)
    {
    // Test failure if not latest EC Version schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="BisCore" version="1.0" prefix="bis"/>
            <ECClass typeName="TestClass" isDomainClass="true">
                <BaseClass>bis:Element</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="TestRelationship">
                <Source cardinality="(1,1)" polymorphic="true">
                    <Class class="TestClass"/>
                </Source>
            </ECRelationshipClass>
        </ECSchema>)xml";
    InitBisContextWithSchemaXml(schemaXml);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema)) << "TestSchema validated successfully even though it is not a valid EC" << ECSchema::GetECVersionString(ECVersion::Latest) << " schema";
    }

    // Test failure to validate schema who ECXML version is not the latest ECXML version.
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="BisCore" version="1.0" prefix="bis"/>
            <ECClass typeName="TestClass" isDomainClass="true">
                <BaseClass>bis:Element</BaseClass>
            </ECClass>
        </ECSchema>)xml";
    InitBisContextWithSchemaXml(schemaXml);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema)) << "TestSchema failed to validate successfully even though it is a valid EC" << ECSchema::GetECVersionString(ECVersion::Latest) << " schema due to its xml version not being the latest";
    }
    // Test successfully validates latest schema
    {
    Utf8String schemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";

    InitBisContextWithSchemaXml(schemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema)) << "TestSchema validates successfully as it is a valid EC" << ECSchema::GetECVersionString(ECVersion::Latest) << " schema";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Dan.Perlman                          05/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, TestSchemaStandardReferences)
    {
    auto const CheckStandardAsReference = [](ECSchemaPtr schema, Utf8CP schemaName, ECSchemaReadContextPtr context, ECSchemaValidator* const validator, bool shouldPassValidation, Utf8CP message) -> void
        {
        SchemaKey refKey = SchemaKey(schemaName, 1, 0);
        ECSchemaPtr refSchema = context->LocateSchema(refKey, SchemaMatchType::LatestWriteCompatible);
        ASSERT_TRUE(refSchema.IsValid());
        schema->AddReferencedSchema(*refSchema.get());
        EXPECT_TRUE(shouldPassValidation == validator->Validate(*schema)) << message;
        schema->RemoveReferencedSchema(*refSchema);
        };

    // Test unsuccessful validation of reference to standard schema
    {
    Utf8String schemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "   <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "   <ECEntityClass typeName='TestClass'>"
        "       <BaseClass>bis:Element</BaseClass>"
        "   </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(schemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    for (auto oldSchemaName : oldStandardSchemaNames)
        CheckStandardAsReference(schema, oldSchemaName, context, &validator, false, "Old standard schemas are used as a reference. Validation should fail.");
    for (auto newSchemaName : newStandardSchemaNames)
        CheckStandardAsReference(schema, newSchemaName, context, &validator, true, "New standard schemas are used as a reference. Validation should succeed.");
    }
    // Use an updated ECDbMap schema as a reference
    {
    Utf8String refXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ECDbMap' alias='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "   <ECEntityClass typeName='TestClass'/>"
        "</ECSchema>";
    ECSchemaPtr refSchema;
    ECSchema::ReadFromXmlString(refSchema, refXml.c_str(), *context);

    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='01.00.00' alias='bis'/>"
        "    <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ref'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema)) << "Should succeed validation as the referenced schema is the latest version of ECDbMap";
    }
    {
    Utf8String refXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ECv3ConversionAttributes' namespacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='TestClass'/>"
        "</ECSchema>";
    
    InitBisContextWithSchemaXml(refXml.c_str());

    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='01.00.00' alias='bis'/>"
        "    <ECSchemaReference name='ECv3ConversionAttributes' version='01.00.00' alias='ref'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema)) << "Should fail validation as the ECv3ConversionAttributes schema is only allowed as a reference if the referencing schema is dynamic.";
    }

    // Test ECv3ConversionAttributes exception to EC2 schema rule.
    {
    Utf8String refXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ECv3ConversionAttributes' namespacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='TestClass'/>"
        "</ECSchema>";
    
    InitBisContextWithSchemaXml(refXml.c_str());

    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='01.00.00' alias='bis'/>"
        "    <ECSchemaReference name='ECv3ConversionAttributes' version='01.00.00' alias='ref'/>"
        "    <ECCustomAttributes>"
        "        <DynamicSchema xmlns='CoreCustomAttributes.01.00.00'/>"
        "    </ECCustomAttributes>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema)) << "Should succeed validation as the ECv3ConversionAttributes schema is an exception to the rule that all reference schemas must be at least 3.1. It also must be on a dynamic schema.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              01/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaValidatorTests, TestSchemasWithNameContainingDynamicApplyDynamicSchemaCA)
    {
    // schemaName does not contain "dynamic" and does not need the DynamicSchema custom attribute.
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='FunkMachine' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema));
    }
    // schemaName contains "Dynamic" and schema has the DynamicSchema custom attribute.
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='DynamicFunkMachine' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECCustomAttributes>"
        "        <DynamicSchema xmlns='CoreCustomAttributes.01.00.00'/>"
        "    </ECCustomAttributes>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema));
    }
    // schemaName contains "dynamic" and does not have the DynamicSchema custom attribute.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='dynamicFunkMachine' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    // schemaName contains "dYnAmic" and does not have the DynamicSchema custom attribute.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='dYnAmicFuNkMAcHinE' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    // schemaName contains "Dynamic" and does not have the DynamicSchema custom attribute.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='DynamicFunkMachine' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    // schemaName contains "Dyanamic" as part of the word "Dynamically" and does not have the DynamicSchema custom attribute.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='SomeDynamicallyGeneratedSchema' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    // schemaName contains "Dyanamic" surrounded by underscores and does not have the DynamicSchema custom attribute.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='Some_Dynamic_Schema' alias='fnk' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              01/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaValidatorTests, RootEntityClassesMustDeriveFromBisHierarchy)
    {
    // Entity class derives from a bis element.
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='GoodTestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema));
    }
    // Entity class "DerivedTestClass" derives from an entity class "BaseTestClass" that derives from a bis element.
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseTestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DerivedTestClass'>"
        "        <BaseClass>BaseTestClass</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_TRUE(validator.Validate(*schema));
    }
    // Entity class written as a single tag does not derive from a bis element.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BadTestClass'/>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    // Entity class written with an opening and closing tag does not derive from a bis element.
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BadTestClass'/>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*schema));
    }
    // Entity class is a derived class but does not derive from a bis element.
    {
    Utf8String refSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ECDbMap' alias='ref' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='BaseTestClass'/>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(refSchemaXml.c_str());
    
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ref'/>"
        "    <ECEntityClass typeName='BadDerivedTestClass'>"
        "        <BaseClass>ref:BaseTestClass</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr badSchema;
    ECSchema::ReadFromXmlString(badSchema, badSchemaXml.c_str(), *context);
    ASSERT_TRUE(badSchema.IsValid());
    EXPECT_TRUE(badSchema->IsECVersion(ECVersion::Latest));
    EXPECT_FALSE(validator.Validate(*badSchema));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Dan.Perlman                          04/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, MixinClassMayNotOverrideInheritedMixinProperty)
    {
    // Test that a mixin class may not override an inherited property from a mixin class
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin0, "Mixin0", *entity0));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin1, "Mixin1", *entity0));
    ASSERT_EQ(ECObjectsStatus::Success, mixin0->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_EQ(ECObjectsStatus::Success, mixin1->AddBaseClass(*mixin0));

    ASSERT_TRUE(validator.Validate(*schema)) << "Mixin property does not override anything so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, mixin1->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Mixin overrides an inherited mixin property so validation should fail";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, MixinClassMayNotOverrideInheritedEntityProperty)
    {
    // Test that a mixin class may not override an inherited property from an entity class
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP mixin0;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "Entity1"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*entity0));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin0, "Mixin0", *entity1));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*mixin0));

    ASSERT_TRUE(validator.Validate(*schema)) << "Mixin property does not override anything so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, mixin0->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Mixin overrides an inherited entity property so validation should fail";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Dan.Perlman                          06/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, BisCoreMultiAspectTests)
    {
    // Element Aspect Relationship Tests
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='Abstract' description='ElementMultiAspect Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='MultiAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "There is no relationship, so validation should fail";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestMultiAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsMultiAspects' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Missing base class in TestRelationship. Validation should fail.";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestMultiAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsUniqueAspect' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <BaseClass>ElementOwnsUniqueAspect</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Target constraint class is ElementMultiAspect, so validation should fail.";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='Abstract' description='ElementMultiAspect Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='MultiAspect' modifier='Abstract' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "There is no relationship but the modifier is abstract, so validation should succeed";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='Schema1' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='Abstract' description='ElementMultiAspect Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MultiAspect' modifier='Abstract' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>bis:IParentElement</BaseClass>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "This not a bis schema, so validation should succeed as this rule does not apply";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestMultiAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsMultiAspects' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <BaseClass>ElementOwnsMultiAspects</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='false'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='false'>"
        "            <Class class='TestMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "BisCore example of a valid multi aspect relationship. Validation should succeed";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='None' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementMultiAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestMultiAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DerivedTestMultiAspect' modifier='None' displayLabel='Derived Test Element Multi-Aspect' description='A Derived Element Multi-Aspect Test Description'>"
        "        <BaseClass>TestMultiAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsMultiAspects' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <BaseClass>ElementOwnsMultiAspects</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestMultiAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Aspect relationship is polymorphic but is supported by MultiAspect, so validation should succeed.";
    }
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              01/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaValidatorTests, BisCoreUniqueAspectTests)
    {
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='Abstract' description='ElementUniqueAspect Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='MultiAspect' modifier='None' displayLabel='Element Unique-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "There is no relationship, so validation should fail";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestUniqueAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsUniqueAspect' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Missing base class in TestRelationship. Validation should fail.";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestUniqueAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsUniqueAspect' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <BaseClass>ElementOwnsUniqueAspect</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Target constraint class is ElementUniqueAspect, so validation should fail.";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='Abstract' description='ElementMultiAspect Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='MultiAspect' modifier='Abstract' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "There is no relationship but the modifier is abstract, so validation should succeed";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='Schema1' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='Abstract' description='ElementUniqueAspect Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ElementAspect' modifier='None' displayLabel='Element Aspect' description='Element Aspect Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MultiAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>bis:IParentElement</BaseClass>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "This not a bis schema, so validation should succeed as this rule does not apply";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Aspect Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestUniqueAspect' modifier='None' displayLabel='Test Element Multi-Aspect' description='An Element Multi-Aspect Test Description'>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsUniqueAspect' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <BaseClass>ElementOwnsUniqueAspect</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='false'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='false'>"
        "            <Class class='TestUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "BisCore example of a valid multi aspect relationship. Validation should succeed";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECEntityClass typeName='Element' modifier='Abstract' description='Element Description'/>"
        "    <ECEntityClass typeName='ElementAspect' modifier='Abstract' displayLabel='Element Aspect' description='Element Aspect Description'/>"
        "    <ECEntityClass typeName='ElementUniqueAspect' modifier='None' displayLabel='Element Multi-Aspect' description='An Element Multi-Unique Description'>"
        "        <BaseClass>ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestUniqueAspect' modifier='None' displayLabel='Test Element Unique-Aspect' description='An Element Multi-Unique Test Description'>"
        "        <BaseClass>ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DerivedTestUniqueAspect' modifier='None' displayLabel='Test Element Unique-Aspect Derived' description='An Element Multi-Unique Test Description'>"
        "        <BaseClass>TestUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ElementOwnsUniqueAspect' strength='embedding' modifier='None'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='ElementUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' strength='embedding' modifier='None'>"
        "        <BaseClass>ElementOwnsUniqueAspect</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='Element'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestUniqueAspect'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "BisCore example of a valid unique aspect relationship. Validation should succeed";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Carole.MacDonald                    07/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, EntityClassMayNotInheritFromCertainBisClasses)
    {
    // Class may not implement both bis:IParentElement and bis:ISubModeledElement
    Utf8String bisSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>" +
            R"xml(<ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA"/>

            <ECEntityClass typeName="IParentElement" modifier="Abstract" description="IParentElement Description">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>

            <ECEntityClass typeName="ISubModeledElement" modifier="Abstract" description="IParentElement Description">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>

            <ECEntityClass typeName="Element" modifier="Abstract" description="Element description"/>
         </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, bisSchemaXml.c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "BisCore succeeds validation";

    Utf8String badBisElementXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BadSchemaThatUsesBis' alias='bis' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BadClass' modifier='Abstract' description='BadClass Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <BaseClass>bis:IParentElement</BaseClass>"
        "        <BaseClass>bis:ISubModeledElement</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr schema2;
    ECSchema::ReadFromXmlString(schema2, badBisElementXml.c_str(), *context);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_FALSE(validator.Validate(*schema2)) << "Schema implements both IParentElement and ISubModeledElement so validation should fail.";

    Utf8String goodBisElementXml1 = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchemaThatUsesBis1' alias='bis' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BadClass' modifier='Abstract' description='BadClass Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <BaseClass>bis:IParentElement</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr schema3;
    ECSchema::ReadFromXmlString(schema3, goodBisElementXml1.c_str(), *context);
    ASSERT_TRUE(schema3.IsValid());
    ASSERT_TRUE(validator.Validate(*schema3)) << "Schema implements only IParentElement so validation should succeed.";

    Utf8String goodBisElementXml2 = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchemaThatUsesBis2' alias='bis' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BadClass' modifier='Abstract' description='BadClass Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <BaseClass>bis:ISubModeledElement</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr schema4;
    ECSchema::ReadFromXmlString(schema4, goodBisElementXml2.c_str(), *context);
    ASSERT_TRUE(schema4.IsValid());
    ASSERT_TRUE(validator.Validate(*schema4)) << "Schema implements only ISubModeledElement so validation should succeed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Colin.Kerr                             07/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, DoNotAllowPropertiesOfTypeLong)
    {
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClassBad'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='PropNameId' typeName='long'/>"
        "    </ECEntityClass>"
        "</ECSchema>";

    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as the property name ends in 'Id' and the type is 'long'";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='SourceClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TargetClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationshipBad' strength='embedding' modifier='None'>"
        "        <ECProperty propertyName='PropNameId' typeName='long'/>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='SourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TargetClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as the property name ends in 'Id' and the type is 'long'";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClassGood1'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='PropName' typeName='long'/>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as the property type is 'long'";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClassGood2'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='PropNameId' typeName='double'/>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as the property name ends in 'Id' but is not type 'long'";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='SourceClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TargetClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECNavigationProperty propertyName='NavProp' relationshipName='TestRelationshipGood' direction='backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationshipGood' strength='embedding' modifier='None'>"
        "        <ECProperty propertyName='PropNameId' typeName='double'/>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='SourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TargetClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());

    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as the property name ends in 'Id' but is not type 'long'";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='SourceClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TargetClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationshipGood' strength='embedding' modifier='None'>"
        "        <ECProperty propertyName='PropertyNameiD' typeName='long'/>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='SourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TargetClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as the property type is long";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='TestSchemaStruct' alias='tss' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECStructClass typeName='AStruct'>"
        "        <ECProperty propertyName='Banana' typeName='long'/>"
        "    </ECStructClass>"
        "</ECSchema>";

    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as a struct property has type long";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Carole.MacDonald                    07/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, RelationshipClassConstraintMayNotBeAbstractIfOnlyOneConcreteConstraint)
    {
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ConstraintTestSchemaFail' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='Base' strength='referencing' modifier='Abstract'>"
        "        <Source multiplicity='(0..*)' roleLabel='refers to' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is referenced by' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='TestRelationship' description='Test description' displayLabel='Test label' modifier='None' strength='referencing'>"
        "        <BaseClass>Base</BaseClass>"
        "        <Source multiplicity='(0..1)' roleLabel='refers to' polymorphic='true' abstractConstraint='TestClass'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is referenced by' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "There is an abstract constraint and only one constraint class in source and target so validation should fail";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='AbstractTestSchemaSucceed' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestClass2'>"
        "        <BaseClass>TestClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='Base' strength='referencing' modifier='Abstract'>"
        "        <Source multiplicity='(0..*)' roleLabel='refers to' polymorphic='true' abstractConstraint='BaseClass'>"
        "            <Class class='BaseClass'/>"
        "            <Class class='TestClass2'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is referenced by' polymorphic='true' abstractConstraint='BaseClass'>"
        "            <Class class='BaseClass'/>"
        "            <Class class='TestClass2'/>"
        "        </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='GoodTestRelationship' description='Test description' displayLabel='Test label' modifier='None' strength='referencing'>"
        "        <BaseClass>Base</BaseClass>"
        "        <Source multiplicity='(0..1)' roleLabel='refers to' polymorphic='true' abstractConstraint='TestClass'>"
        "            <Class class='TestClass'/>"
        "            <Class class='TestClass2'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is referenced by' polymorphic='true' abstractConstraint='TestClass'>"
        "            <Class class='TestClass'/>"
        "            <Class class='TestClass2'/>"
        "        </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Abstract constraints are defined locally in source and target so validation should succeed";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Carole.MacDonald                    07/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, RelationshipClassMayNotHaveHoldingStrength)
    {
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='holding' modifier='Sealed'>"
        "    <Source multiplicity='(1..1)' roleLabel='read from source to target' polymorphic='true'>"
        "        <Class class='TestClass'/>"
        "    </Source>"
        "    <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "        <Class class='TestClass'/>"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as strength attribute must not be set to 'holding'";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' modifier='Sealed'>"
        "        <Source multiplicity='(1..1)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should pass validation as strength attribute is set to 'embedding'";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Carole.MacDonald                    07/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, RelationshipClassEmbeddingStrengthTests)
    {
    // Test forward direction
    // Source = 0..*
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='forward' modifier='Sealed'>"
        "        <Source multiplicity='(0..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as source multiplicity upper bound is greater than 1 while strength is embedding and forward";
    }
    // Source = 1..*
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='forward' modifier='Sealed'>"
        "        <Source multiplicity='(1..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as source multiplicity upper bound is greater than 1 while strength is embedding and forward";
    }
    // Source = 0..1
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='forward' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' roleLabel='read from source to target' polymorphic='true'>"
        "        <Class class='TestClass'/>"
        "    </Source>"
        "    <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "        <Class class='TestClass'/>"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as source multiplicity upper bound is not greater than 1 while strength is embedding and forward";
    }
    // Source = 1..1
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='forward' modifier='Sealed'>"
        "        <Source multiplicity='(1..1)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as source multiplicity upper bound is not greater than 1 while strength is embedding and forward";
    }
    // Test backward direction
    // Target = 0..*
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='backward' modifier='Sealed'>"
        "        <Source multiplicity='(0..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as target multiplicity upper bound is greater than 1 while strength is embedding and backward";
    }
    // Target = 1..*
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='backward' modifier='Sealed'>"
        "        <Source multiplicity='(1..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(1..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as target multiplicity upper bound is greater than 1 while strength is embedding and backward";
    }
    // Target = 0..1
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='backward' modifier='Sealed'>"
        "        <Source multiplicity='(0..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..1)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as target multiplicity upper bound is not greater than 1 while strength is embedding and backward";
    }
    // Target = 1..1
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' strengthDirection='backward' modifier='Sealed'>"
        "        <Source multiplicity='(0..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(1..1)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as target multiplicity upper bound is not greater than 1 while strength is embedding and backward";
    }
    // No direction given, so forward is assumed
    // Source = 0..*
    {
    Utf8String badSchemaNoDirection = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' modifier='Sealed'>"
        "        <Source multiplicity='(0..*)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaNoDirection.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as direction is assumed to be forward with embedding strength, with multiplicity in source greater than 1";
    }
    // Source = 0..1
    {
    Utf8String goodSchemaNoDirection = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='StandardSchemaReferenced' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='ExampleRelationship' strength='embedding' modifier='Sealed'>"
        "        <Source multiplicity='(0..1)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaNoDirection.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as direction is assumed to be forward, with multiplicity equal to 1";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Aurora.Lane                         10/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, RelationshipClassMayNotHaveElementAspectTargets)
    {
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ElementOwnsAspectSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElementAspectClass'>"
        "        <BaseClass>bis:ElementUniqueAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding'>"
        "        <BaseClass>bis:ElementOwnsUniqueAspect</BaseClass>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestElementAspectClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "If Relationship is based on ElementOwns[...]Aspect, it should validate regardless of ElementAspects";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ElementAspectTargetSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElementAspectClass'>"
        "        <BaseClass>bis:ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestElementAspectClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "If ElementAspect is target of relationship, schema is not valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ElementAspectTargetSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElementAspectClass'>"
        "        <BaseClass>bis:ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestElementAspectClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "If ElementAspect is source of forward relationship, schema is valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='NoElementAspectTargetSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestTargetClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding'>"
        "        <Source multiplicity='(1..1)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestTargetClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "No ElementAspects in relationship, schema is valid";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ElementAspectSourceSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElementAspectClass'>"
        "        <BaseClass>bis:ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding' strengthDirection='Backward'>"
        "        <Source multiplicity='(0..*)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestElementAspectClass'/>"
        "        </Source>"
        "        <Target multiplicity='(1..1)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "If ElementAspect is source of backwards relationship, schema is not valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ElementAspectSourceSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElementAspectClass'>"
        "        <BaseClass>bis:ElementAspect</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding' strengthDirection='Backward'>"
        "        <Source multiplicity='(0..*)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(1..1)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestElementAspectClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "If ElementAspect is target of backwards relationship, schema is valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='NoElementAspectSourceSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestSourceClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestTargetClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='TestRelationship' modifier='None' strength='embedding' strengthDirection='Backward'>"
        "        <Source multiplicity='(0..*)' roleLabel='owns' polymorphic='true'>"
        "            <Class class='TestSourceClass'/>"
        "        </Source>"
        "        <Target multiplicity='(1..1)' roleLabel='is owned by' polymorphic='true'>"
        "            <Class class='TestTargetClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "No ElementAspects in backwards relationship, schema is valid";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Carole.MacDonald                    07/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, EmbeddingRelationshipsShouldNotContainHasInClassName)
    {
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BadSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelationshipHasBadString' strength='embedding' modifier='Sealed'>"
        "        <Source multiplicity='(0..1)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as relationship is embedding and contains 'Has'";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RelationshipHasPotentiallyBadString' strength='referencing' modifier='Sealed'>"
        "        <Source multiplicity='(0..1)' roleLabel='read from source to target' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Source>"
        "        <Target multiplicity='(0..*)' roleLabel='read from target to source' polymorphic='true'>"
        "            <Class class='TestClass'/>"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as relationship is 'referncing', not 'embedding'";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Dan.Perlman                          06/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, KindOfQuantityShouldUseSIPersistenceUnits)
    {
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BadSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECSchemaReference name='Units' version='01.00.00' alias='u'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <KindOfQuantity typeName='BadKOQ' displayLabel='OFFSET' persistenceUnit='u:IN' relativeError='1e-2' />"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as persistence unit is a UCUSTOM unit, 'IN', not an SI unit";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BadSchema2' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECSchemaReference name='Units' version='01.00.00' alias='u'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <KindOfQuantity typeName='BadKOQ2' displayLabel='LENGTH' persistenceUnit='u:US_SURVEY_IN' relativeError='1e-3' />"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as persistence unit is a USSURVEY unit, 'US_SURVEY_IN', not an SI unit";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BadSchema3' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECSchemaReference name='Units' version='01.00.00' alias='u'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <KindOfQuantity typeName='BadKOQ3' displayLabel='DEPTH' persistenceUnit='u:NAUT_MILE' relativeError='1e-3' />"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as persistence unit is a MARITIME unit, 'NAUT_MILE', not an SI unit";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECSchemaReference name='Units' version='01.00.00' alias='u'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <KindOfQuantity typeName='GoodKOQ' displayLabel='LENGTH' persistenceUnit='u:M' relativeError='1e-1' />"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as persistence unit is an SI unit, 'M'";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema2' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECSchemaReference name='Units' version='01.00.00' alias='u'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "    <KindOfQuantity typeName='GoodKOQ2' displayLabel='OFFSET' persistenceUnit='u:CM' relativeError='1e-4' />"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as persistence unit is an METRIC unit, 'CM'";
    }
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                       Colin.Kerr                      09/2017
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidatorTests, StructsShouldNotHaveBaseClasses)
    {
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECStructClass typeName='BaseClass'>"
        "        <ECProperty propertyName='Length' typeName='double'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='DerivedClass'>"
        "        <ECProperty propertyName='Length' typeName='double' />"
        "    </ECStructClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation since structs do not have base classes";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECStructClass typeName='BaseClass'>"
        "        <ECProperty propertyName='Length' typeName='double'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='DerivedClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='Length' typeName='double'/>"
        "    </ECStructClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation because structs have base classes";
    }
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                       Colin.Kerr                      09/2017
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaValidatorTests, CustomAttributesShouldNotHaveBaseClasses)
    {
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECCustomAttributeClass typeName='BaseClass'>"
        "        <ECProperty propertyName='Length' typeName='double'/>"
        "    </ECCustomAttributeClass>"
        "    <ECCustomAttributeClass typeName='DerivedClass'>"
        "        <ECProperty propertyName='Length' typeName='double' />"
        "    </ECCustomAttributeClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation since custom attributes do not have base classes";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECCustomAttributeClass typeName='BaseClass'>"
        "        <ECProperty propertyName='Length' typeName='double'/>"
        "    </ECCustomAttributeClass>"
        "    <ECCustomAttributeClass typeName='DerivedClass'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='Length' typeName='double'/>"
        "    </ECCustomAttributeClass>"
        "</ECSchema>";
    InitContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation because custom attributes have base classes";
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, TestSchemaReferenceVersion)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SchemaReferencedVersion" alias="srv" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:Element</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="RefSchema" version="01.00.00" alias="ref"/>
            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:Element</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    // Test successful validation of EC 3.1 reference
    {
    InitBisContextWithSchemaXml(schemaXml);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    // Use an EC 3.1 schema as a reference
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RefSchema" alias="ref" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
        </ECSchema>)xml";
    ECSchemaPtr refSchema;
    ECSchema::ReadFromXmlString(refSchema, refXml, *context);

    ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    ECSchemaValidator validator = ECSchemaValidator();
    EXPECT_TRUE(validator.Validate(*schema)) << "Should succeed validation as the referenced schema uses EC 3.1";
    }

    // Test unsuccessful validation of EC 3.0 reference
    {
    InitBisContextWithSchemaXml(schemaXml);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    // Use an EC 3.0 schema as a reference
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RefSchema" alias="ref" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
            <ECEntityClass typeName="TestClass"/>
        </ECSchema>)xml";
    ECSchemaPtr refSchema;
    ECSchema::ReadFromXmlString(refSchema, refXml, *context);

    ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    ECSchemaValidator validator = ECSchemaValidator();
    EXPECT_FALSE(validator.Validate(*schema)) << "Should fail validation as the referenced schema uses EC 3.0";
    }

    // Test unsuccessful validation of EC 2.0 reference
    {
    InitBisContextWithSchemaXml(schemaXml);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    // Use an EC 2.0 schema as a reference
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RefSchema" alias="ref" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECEntityClass typeName="TestClass"/>
        </ECSchema>)xml";
    ECSchemaPtr refSchema;
    ECSchema::ReadFromXmlString(refSchema, refXml, *context);

    ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    ECSchemaValidator validator = ECSchemaValidator();
    EXPECT_FALSE(validator.Validate(*schema)) << "Should fail validation as the referenced schema uses EC 2.0";
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                       04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, EntityClassMayOverrideInheritedMixinProperty)
    {
    // Test that an entity class may not override a property inherited from mixin class
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity;
    ECEntityClassP mixin;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity, "Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity->AddBaseClass(*bisEntity));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin, "Mixin0", *entity));
    ASSERT_EQ(ECObjectsStatus::Success, mixin->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_EQ(ECObjectsStatus::Success, entity->AddBaseClass(*mixin));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class inherits a property from mixin class so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, entity->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class overrides a property inherited from mixin class, but validation should still succeed";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, EntityClassMayNotInheritFromMultipleEntityBaseClasses)
    {
    // Test that an entity class may not derive from multiple base entity classes
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP derivedEntity;
    ECEntityClassP baseEntity1;
    ECEntityClassP baseEntity2;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(derivedEntity, "DerivedEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(baseEntity1, "BaseEntity1"));
    ASSERT_EQ(ECObjectsStatus::Success, baseEntity1->AddBaseClass(*bisEntity));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(baseEntity2, "BaseEntity2"));
    ASSERT_EQ(ECObjectsStatus::Success, baseEntity2->AddBaseClass(*bisEntity));

    ASSERT_EQ(ECObjectsStatus::Success, derivedEntity->AddBaseClass(*baseEntity1));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class derives from one base entity class so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, derivedEntity->AddBaseClass(*baseEntity2));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class derives from multiple base entity classes so validation should fail";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, EntityClassMayNotInheritPropertyFromMultipleMixinClasses)
    {
    // Test that an entity class may not inherit a property from multiple mixins
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin0, "Mixin0", *entity0));
    ASSERT_EQ(ECObjectsStatus::Success, mixin0->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*mixin0));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin1, "Mixin1", *entity0));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*mixin1));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class inherits property from one mixin class so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, mixin1->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class inherits property from multiple mixin classes so validation should fail";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
++---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, DiamondPatternInheritedProperty)
    {
    // Test the diamond pattern, mixin class with a property is inherited by an entity class and a mixin class, which are both inherited by an entity class
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin0, "Mixin0", *entity0));
    ASSERT_EQ(ECObjectsStatus::Success, mixin0->CreatePrimitiveProperty(prop, "P1"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*mixin0));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "Entity1"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*entity0));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixin1, "Mixin1", *entity1));
    ASSERT_EQ(ECObjectsStatus::Success, mixin1->AddBaseClass(*mixin0));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*mixin1));

    ASSERT_TRUE(validator.Validate(*schema)) << "Mixin property is not overridden so validation should succeed";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, EntityClassesMayNotHaveTheSameDisplayLabel)
    {
    {
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    EC_ASSERT_SUCCESS(bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 1, 1));
    EC_ASSERT_SUCCESS(schema->AddReferencedSchema(*bisSchema));
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entity0, "E0"));
    EC_ASSERT_SUCCESS(entity0->AddBaseClass(*bisEntity));
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entity1, "E1"));
    EC_ASSERT_SUCCESS(entity1->AddBaseClass(*bisEntity));
    
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as no display labels are defined";

    ASSERT_EQ(ECObjectsStatus::Success, entity1->SetDisplayLabel("E0"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as defined display label conflicts with a class name without a defined display label";

    ASSERT_EQ(ECObjectsStatus::Success, entity0->SetDisplayLabel("Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->SetDisplayLabel("Entity1"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as defined display labels are different";

    ASSERT_EQ(ECObjectsStatus::Success, entity0->SetDisplayLabel("DuplicateLabel"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->SetDisplayLabel("DuplicateLabel"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Should fail validation as defined display labels are the same";
    }

    // Dynamic schemas currently have an exception to this rule for the iModelBridge workflow.
    {
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    EC_ASSERT_SUCCESS(bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 1, 1));
    EC_ASSERT_SUCCESS(schema->AddReferencedSchema(*bisSchema));
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entity0, "E0"));
    EC_ASSERT_SUCCESS(entity0->AddBaseClass(*bisEntity));
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entity1, "E1"));
    EC_ASSERT_SUCCESS(entity1->AddBaseClass(*bisEntity));

    EC_ASSERT_SUCCESS(schema->AddReferencedSchema(*CoreCustomAttributeHelper::GetSchema()));
    
    ECClassCP dynamicSchemaClass = CoreCustomAttributeHelper::GetClass("DynamicSchema");
    StandaloneECInstancePtr dynamicSchemaInstance = dynamicSchemaClass->GetDefaultStandaloneEnabler()->CreateInstance();
    schema->SetCustomAttribute(*dynamicSchemaInstance.get());
    ASSERT_TRUE(schema->IsDynamicSchema());
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as no display labels are defined";

    ASSERT_EQ(ECObjectsStatus::Success, entity1->SetDisplayLabel("E0"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation since this is a dynamic schema";

    ASSERT_EQ(ECObjectsStatus::Success, entity0->SetDisplayLabel("Entity0"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->SetDisplayLabel("Entity1"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation as defined display labels are different";

    ASSERT_EQ(ECObjectsStatus::Success, entity0->SetDisplayLabel("DuplicateLabel"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->SetDisplayLabel("DuplicateLabel"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Should succeed validation since this is a dynamic schema";
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, PropertiesMayNotHaveTheSameDisplayLabelAndCategory)
    {
    // Test that an entity class may not inherit a property from multiple mixins
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity;
    ECSchemaPtr schema;
    ECEntityClassP entity;
    PrimitiveECPropertyP prop1;
    PrimitiveECPropertyP prop2;
    PropertyCategoryP cat1;
    PropertyCategoryP cat2;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    EC_ASSERT_SUCCESS(bisSchema->CreateEntityClass(bisEntity, "BisEntity"));

    {
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 1, 1));
    EC_ASSERT_SUCCESS(schema->AddReferencedSchema(*bisSchema));
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entity, "Entity"));
    EC_ASSERT_SUCCESS(schema->CreatePropertyCategory(cat1, "category1"));
    EC_ASSERT_SUCCESS(schema->CreatePropertyCategory(cat2, "category2"));
    EC_ASSERT_SUCCESS(entity->AddBaseClass(*bisEntity));

    EC_ASSERT_SUCCESS(entity->CreatePrimitiveProperty(prop1, "P1"));
    EC_ASSERT_SUCCESS(entity->CreatePrimitiveProperty(prop2, "P2"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with different names so validation should succeed";

    EC_ASSERT_SUCCESS(prop2->SetDisplayLabel("P1"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class has two properties with the same display label and no defined category so validation should fail";

    EC_ASSERT_SUCCESS(prop1->SetDisplayLabel("Property1"));
    EC_ASSERT_SUCCESS(prop2->SetDisplayLabel("Property2"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with different display labels so validation should succeed";

    EC_ASSERT_SUCCESS(prop1->SetDisplayLabel("DuplicateLabel"));
    EC_ASSERT_SUCCESS(prop2->SetDisplayLabel("DuplicateLabel"));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class has two properties with the same display label and no defined category so validation should fail";

    EC_ASSERT_SUCCESS(prop1->SetCategory(cat1));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label but different categories so validation should succeed";

    EC_ASSERT_SUCCESS(prop2->SetCategory(cat1));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class has two properties with the same display label and category so validation should fail";

    EC_ASSERT_SUCCESS(prop2->SetCategory(cat2));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label but different categories so validation should succeed";
    }

    // Dynamic schemas currently have an exception to this rule for the iModelBridge workflow.
    {
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 1, 1));
    EC_ASSERT_SUCCESS(schema->AddReferencedSchema(*bisSchema));
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entity, "Entity"));
    EC_ASSERT_SUCCESS(schema->CreatePropertyCategory(cat1, "category1"));
    EC_ASSERT_SUCCESS(schema->CreatePropertyCategory(cat2, "category2"));
    EC_ASSERT_SUCCESS(entity->AddBaseClass(*bisEntity));

    EC_ASSERT_SUCCESS(schema->AddReferencedSchema(*CoreCustomAttributeHelper::GetSchema()));

    ECClassCP dynamicSchemaClass = CoreCustomAttributeHelper::GetClass("DynamicSchema");
    StandaloneECInstancePtr dynamicSchemaInstance = dynamicSchemaClass->GetDefaultStandaloneEnabler()->CreateInstance();
    schema->SetCustomAttribute(*dynamicSchemaInstance.get());
    ASSERT_TRUE(schema->IsDynamicSchema());

    EC_ASSERT_SUCCESS(entity->CreatePrimitiveProperty(prop1, "P1"));
    EC_ASSERT_SUCCESS(entity->CreatePrimitiveProperty(prop2, "P2"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with different names so validation should succeed";

    EC_ASSERT_SUCCESS(prop2->SetDisplayLabel("P1"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label and no defined category so validation should fail";

    EC_ASSERT_SUCCESS(prop1->SetDisplayLabel("Property1"));
    EC_ASSERT_SUCCESS(prop2->SetDisplayLabel("Property2"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with different display labels so validation should succeed";

    EC_ASSERT_SUCCESS(prop1->SetDisplayLabel("DuplicateLabel"));
    EC_ASSERT_SUCCESS(prop2->SetDisplayLabel("DuplicateLabel"));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label and no defined category so validation should fail";

    EC_ASSERT_SUCCESS(prop1->SetCategory(cat1));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label but different categories so validation should succeed";

    EC_ASSERT_SUCCESS(prop2->SetCategory(cat1));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label and category so validation should fail";

    EC_ASSERT_SUCCESS(prop2->SetCategory(cat2));
    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class has two properties with the same display label but different categories so validation should succeed";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Aurora.Lane                         10/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, PropertyExtendedTypesMustBeOnApprovedList)
    {
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='NoETypeSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='TestProperty' typeName='string'>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Properties without ExtendedTypes are valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='JsonSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='TestProperty' typeName='string' extendedTypeName='Json'>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Properties of ExtendedType 'Json' are valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BeGuidSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='TestProperty' typeName='string' extendedTypeName='BeGuid'>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Properties of ExtendedType 'BeGuid' are valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GeometrySchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='TestProperty' typeName='string' extendedTypeName='GeometryStream'>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Properties of ExtendedType 'GeometryStream' are valid";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GarbageETypeSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECProperty propertyName='TestProperty' typeName='string' extendedTypeName='Garbage'>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Properties of any other ExtendedType are not valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ArrayJsonSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECArrayProperty propertyName='TestProperty' typeName='string' extendedTypeName='Json'>"
        "        </ECArrayProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Array Properties of ExtendedType 'Json' are valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ArrayBeGuidSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECArrayProperty propertyName='TestProperty' typeName='string' extendedTypeName='BeGuid'>"
        "        </ECArrayProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Array Properties of ExtendedType 'BeGuid' are valid";
    }
    {
    Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ArrayGeometrySchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECArrayProperty propertyName='TestProperty' typeName='string' extendedTypeName='GeometryStream'>"
        "        </ECArrayProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(goodSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "Array Properties of ExtendedType 'GeometryStream' are valid";
    }
    {
    Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='ArrayGarbageETypeSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "        <ECArrayProperty propertyName='TestProperty' typeName='string' extendedTypeName='Garbage'>"
        "        </ECArrayProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";
    InitBisContextWithSchemaXml(badSchemaXml.c_str());
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(validator.Validate(*schema)) << "Array Properties of any other ExtendedType are not valid";
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, EntityClassMayNotSubclassPhysicalModel)
    {
    // Class may not subclass bis:PhysicalModel
    Utf8String bisSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>" +
        R"xml(<ECEntityClass typeName="Element" modifier="Abstract" description="Element description"/>
            <ECEntityClass typeName="PhysicalModel" displayLabel="Physical Model" description="PhysicalModel Description" />
         </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, bisSchemaXml.c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(validator.Validate(*schema)) << "BisCore succeeds validation";

    Utf8String goodBisElementXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='GoodSchema' alias='bis' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='GoodClass' modifier='Abstract' description='GoodClass Description'>"
        "        <BaseClass>bis:Element</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr schema2;
    ECSchema::ReadFromXmlString(schema2, goodBisElementXml.c_str(), *context);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_TRUE(validator.Validate(*schema2)) << "Schema does not implement PhysicalModel so validation should succeed.";

    Utf8String badBisElementXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
        "<ECSchema schemaName='BadSchema' alias='bis' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
        "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
        "    <ECEntityClass typeName='BadClass' modifier='Abstract' description='BadClass Description'>"
        "        <BaseClass>bis:PhysicalModel</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";
    ECSchemaPtr schema3;
    ECSchema::ReadFromXmlString(schema3, badBisElementXml.c_str(), *context);
    ASSERT_TRUE(schema3.IsValid());
    ASSERT_FALSE(validator.Validate(*schema3)) << "Schema subclasses PhysicalModel so validation should fail.";

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, NoClassShouldSubclassSpatialLocationModel)
    {
    // Test that an entity class may not subclass bis:SpatialLocationModel
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity, bisSpatialLocationModel;
    ECSchemaPtr schema;
    ECEntityClassP entity0, entity1;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisSpatialLocationModel, "SpatialLocationModel"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "GoodEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class does not subclass bis:SpatialLocationModel, so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "BadEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*bisSpatialLocationModel));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class subclasses bis:SpatialLocationModel, so validation should fail";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, NoClassShouldSubclassInformationRecordModel)
    {
    // Test that an entity class may not subclass bis:InformationRecordModel
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity, bisInformationRecordModel;
    ECSchemaPtr schema;
    ECEntityClassP entity0, entity1;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisInformationRecordModel, "InformationRecordModel"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "GoodEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class does not subclass bis:InformationRecordModel, so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "BadEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*bisInformationRecordModel));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class subclasses bis:InformationRecordModel, so validation should fail";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, NoClassShouldSubclassDocumentListModel)
    {
    // Test that an entity class may not subclass bis:DocumentListModel
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity, bisDocumentListModel;
    ECSchemaPtr schema;
    ECEntityClassP entity0, entity1;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisDocumentListModel, "DocumentListModel"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "GoodEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class does not subclass bis:DocumentListModel, so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "BadEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*bisDocumentListModel));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class subclasses bis:DocumentListModel, so validation should fail";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, NoClassShouldSubclassLinkModel)
    {
    // Test that an entity class may not subclass bis:LinkModel
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity, bisLinkModel;
    ECSchemaPtr schema;
    ECEntityClassP entity0, entity1;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisLinkModel, "LinkModel"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "GoodEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class does not subclass bis:LinkModel, so validation should succeed";
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "BadEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*bisLinkModel));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class subclasses bis:LinkModel, so validation should fail";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Joseph.Urbano                        08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaValidatorTests, NoClassShouldSubclassDefinitionModelWithExceptions)
    {
    // Test that an entity class may not subclass bis:DefinitionModel, except for bis:DictionaryModel and bis:RepositoryModel
    ECSchemaPtr bisSchema;
    ECEntityClassP bisEntity, bisDefinitionModel, bisDictionaryModel, bisRepositoryModel;
    ECSchemaPtr schema;
    ECEntityClassP entity0, entity1;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(bisSchema, "BisCore", "bis", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisEntity, "BisEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisDefinitionModel, "DefinitionModel"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisDictionaryModel, "DictionaryModel"));
    ASSERT_EQ(ECObjectsStatus::Success, bisSchema->CreateEntityClass(bisRepositoryModel, "RepositoryModel"));
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "EntityClassSchema", "ECC", 1, 1, 1));
    ASSERT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*bisSchema));
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity0, "GoodEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity0->AddBaseClass(*bisEntity));

    ASSERT_TRUE(validator.Validate(*schema)) << "Entity class does not subclass bis:DefinitionModel, so validation should succeed";

    ASSERT_EQ(ECObjectsStatus::Success, bisDictionaryModel->AddBaseClass(*bisDefinitionModel));
    ASSERT_TRUE(validator.Validate(*schema)) << "bis:DictionrayModel may subclass bis:DefinitionModel, so validation should succeed";

    ASSERT_EQ(ECObjectsStatus::Success, bisRepositoryModel->AddBaseClass(*bisDefinitionModel));
    ASSERT_TRUE(validator.Validate(*schema)) << "bis:RepositoryModel may subclass bis:DefinitionModel, so validation should succeed";

    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entity1, "BadEntity"));
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*bisDefinitionModel));
    ASSERT_FALSE(validator.Validate(*schema)) << "Entity class subclasses bis:DefinitionModel, so validation should fail";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Aurora.Lane                         10/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, NoModelSubclassMayHaveProperties)
    {
            {
            Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
                "<ECSchema schemaName='BisCore' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
                "    <ECEntityClass typeName='Model' modifier='Abstract'>"
                "        <ECProperty propertyName='ModelTestProp' typeName='int'/>"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='TestClass'>"
                "        <BaseClass>Model</BaseClass>"
                "        <ECProperty propertyName='TestProperty' typeName='int'/>"
                "    </ECEntityClass>"
                "</ECSchema>";
            InitContextWithSchemaXml(goodSchemaXml.c_str());
            ASSERT_TRUE(schema.IsValid());
            ASSERT_TRUE(validator.Validate(*schema)) << "Model Subclasses with properties in BisCore are valid";
            }
            {
            Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
                "<ECSchema schemaName='ModelTestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
                "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
                "    <ECEntityClass typeName='TestClass'>"
                "        <BaseClass>bis:Model</BaseClass>"
                "    </ECEntityClass>"
                "</ECSchema>";
            InitBisContextWithSchemaXml(goodSchemaXml.c_str());
            ASSERT_TRUE(schema.IsValid());
            ASSERT_TRUE(validator.Validate(*schema)) << "Model Subclasses without properties are valid";
            }
            {
            Utf8String badSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
                "<ECSchema schemaName='ModelTestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
                "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
                "    <ECEntityClass typeName='TestClass'>"
                "        <BaseClass>bis:Model</BaseClass>"
                "        <ECProperty propertyName='TestProperty' typeName='int'/>"
                "    </ECEntityClass>"
                "</ECSchema>";
            InitBisContextWithSchemaXml(badSchemaXml.c_str());
            ASSERT_TRUE(schema.IsValid());
            ASSERT_FALSE(validator.Validate(*schema)) << "Model Subclasses with properties are not valid";
            }
            {
            Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
                "<ECSchema schemaName='ModelTestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
                "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
                "    <ECEntityClass typeName='TestClass'>"
                "        <BaseClass>bis:Model</BaseClass>"
                "        <ECProperty propertyName='IsPrivate' typeName='boolean'/>"
                "    </ECEntityClass>"
                "</ECSchema>";
            InitBisContextWithSchemaXml(goodSchemaXml.c_str());
            ASSERT_TRUE(schema.IsValid());
            ASSERT_TRUE(validator.Validate(*schema)) << "Model Subclasses with override properties are valid";
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                         02/2019
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidatorTests, ErrorsForAlreadyReleasedSchemasAreSuppressed)
    {
        {
        // BuildingPhysical
        Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
            "<ECSchema schemaName='BuildingPhysical' alias='bp' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
            "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
            "    <ECEntityClass typeName='BuildingPhysicalModel'>"
            "        <BaseClass>bis:PhysicalModel</BaseClass>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='BuildingTypeDefinitionModel'>"
            "        <BaseClass>bis:DefinitionModel</BaseClass>"
            "    </ECEntityClass>"
            "</ECSchema>";
        InitBisContextWithSchemaXml(goodSchemaXml.c_str());
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(validator.Validate(*schema)) << "BuildingPhysical may violate the model subclassing rule with BuildingPhyscialModel and BuildingTypeDefinitionModel";
        }
        {
        // Raster
        Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
            "<ECSchema schemaName='Raster' alias='raster' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
            "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
            "    <ECEntityClass typeName='RasterModel'>"
            "        <BaseClass>bis:Model</BaseClass>"
            "        <ECProperty propertyName='Clip' typeName='boolean'/>"
            "    </ECEntityClass>"
            "</ECSchema>";
        InitBisContextWithSchemaXml(goodSchemaXml.c_str());
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(validator.Validate(*schema)) << "Raster may violate the model subclass adds property rule with RasterModel property Clip";
        }
        {
        // RoadRailAlignment
        Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
            "<ECSchema schemaName='RoadRailAlignment' alias='rra' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
            "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
            "    <ECEntityClass typeName='AlignmentModel'>"
            "        <BaseClass>bis:SpatialLocationModel</BaseClass>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='ConfigurationModel'>"
            "        <BaseClass>bis:DefinitionModel</BaseClass>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='HorizontalAlignmentModel'>"
            "        <BaseClass>bis:SpatialLocationModel</BaseClass>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='RoadRailCategoryModel'>"
            "        <BaseClass>bis:DefinitionModel</BaseClass>"
            "    </ECEntityClass>"
            "</ECSchema>";
        InitBisContextWithSchemaXml(goodSchemaXml.c_str());
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(validator.Validate(*schema)) << "RoadRailAlignment may violate the model subclass rule with AlignmentModel, ConfigurationModel, HorizontalAlignmentModel, and RoadRailCategoryModel";
        }
        {
        // RoadRailPhyscial
        Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
            "<ECSchema schemaName='RoadRailPhysical' alias='rrp' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
            "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
            "    <ECEntityClass typeName='DesignSpeed' displayLabel='Design Speed'>"
            "        <BaseClass>bis:PhysicalElement</BaseClass>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='DesignSpeedElement' displayLabel='Design Speed'>"
            "        <BaseClass>bis:PhysicalElement</BaseClass>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='RailwayStandardsModel'>"
            "        <BaseClass>bis:DefinitionModel</BaseClass>"
            "    </ECEntityClass>"
            "</ECSchema>";
        InitBisContextWithSchemaXml(goodSchemaXml.c_str());
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(validator.Validate(*schema)) << "RoadRailPhyscial may violate the model subclass rule with RailwayStandardsModel and the same display label rule with DesignSpeed and DesignSpeedElement";
        }
        {
        // ScalableMesh
        Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
            "<ECSchema schemaName='ScalableMesh' alias='sm' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
            "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
            "    <ECEntityClass typeName='ScalableMeshModel'>"
            "        <BaseClass>bis:SpatialModel</BaseClass>"
            "        <ECProperty propertyName='SmModelClips' typeName='boolean'/>"
            "        <ECProperty propertyName='SmGroundCoverages' typeName='boolean'/>"
            "        <ECProperty propertyName='SmModelClipVectors' typeName='boolean'/>"
            "    </ECEntityClass>"
            "</ECSchema>";
        InitBisContextWithSchemaXml(goodSchemaXml.c_str());
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(validator.Validate(*schema)) << "ScalableMesh may violate the model subclass adds property rule with ScalableMeshModel properties SmModelClips, SmGroundCoverages and SmModelClipVectors";
        }
        {
        // BuildingPhysical
        Utf8String goodSchemaXml = Utf8String("<?xml version='1.0' encoding='UTF-8'?>") +
            "<ECSchema schemaName='StructuralPhysical' alias='sp' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML." + ECSchema::GetECVersionString(ECVersion::Latest) + "'>"
            "    <ECSchemaReference name='BisCore' version='1.0.0' alias='bis'/>"
            "    <ECEntityClass typeName='StructuralPhysicalModel'>"
            "        <BaseClass>bis:PhysicalModel</BaseClass>"
            "    </ECEntityClass>"
            "</ECSchema>";
        InitBisContextWithSchemaXml(goodSchemaXml.c_str());
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(validator.Validate(*schema)) << "StructuralPhysical may violate the model subclassing rule with StructuralPhysicalModel";
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
