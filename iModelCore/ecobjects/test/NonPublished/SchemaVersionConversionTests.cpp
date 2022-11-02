/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaVersionConversionTests : ECTestFixture { };

struct ExpectedClassMetaData
    {
    ExpectedClassMetaData(Utf8CP className, ECClassType type, ECClassModifier modifier)
        {
        ClassName = className;
        Type = type;
        Modifier = modifier;
        }
    Utf8CP ClassName;
    ECClassType Type;
    ECClassModifier Modifier;
    };

void VerifySchema(ECSchemaPtr schema, bvector<ExpectedClassMetaData> expectedClasses, bvector<Utf8CP> unexpectedClasses)
    {
    for (auto const& expected : expectedClasses)
        {
        ECClassCP expectedClass = schema->GetClassCP(expected.ClassName);
        ASSERT_NE(nullptr, expectedClass) << "Could not find class '" << expected.ClassName << "' in converted schema";
        ASSERT_EQ(expected.Type, expectedClass->GetClassType()) << "Type of class incorrect for '" << expected.ClassName << "' in converted schema";
        ASSERT_EQ(expected.Modifier, expectedClass->GetClassModifier()) << "Modifer for class incorrect for '" << expected.ClassName << "' in converted schema";
        }
    for (auto const& unexpectedClassName : unexpectedClasses)
        {
        ECClassCP unexpectedClass = schema->GetClassCP(unexpectedClassName);
        ASSERT_EQ(nullptr, unexpectedClass) << "Found class '" << unexpectedClassName << "' but it should have been removed from the converted schema";
        }
    }

void CreateExpectedWithAttributes(bvector<ExpectedClassMetaData>& expectedClasses)
    {
    expectedClasses.push_back(ExpectedClassMetaData("EntityAbstract", ECClassType::Entity, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithJustDomain", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("EntitySealed", ECClassType::Entity, ECClassModifier::Sealed));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithAllFlags", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithDomainAndCa", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithDomainAndStruct", ECClassType::Entity, ECClassModifier::None));

    expectedClasses.push_back(ExpectedClassMetaData("StructAbstract", ECClassType::Struct, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithJustStruct", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("StructSealed", ECClassType::Struct, ECClassModifier::Sealed));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithAllFlags", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithStructAndCa", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithStructAndDomain", ECClassType::Struct, ECClassModifier::None));

    expectedClasses.push_back(ExpectedClassMetaData("CaAbstract", ECClassType::CustomAttribute, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithJustCa", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CaSealed", ECClassType::CustomAttribute, ECClassModifier::Sealed));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithAllFlags", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithCaAndDomain", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithCaAndStruct", ECClassType::CustomAttribute, ECClassModifier::None));

    expectedClasses.push_back(ExpectedClassMetaData("RelationshipAbstract", ECClassType::Relationship, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("RelationshipConcrete", ECClassType::Relationship, ECClassModifier::None));
    }

void CreateExpectedWithOutAttributes(bvector<ExpectedClassMetaData>& expectedClasses)
    {
    expectedClasses.push_back(ExpectedClassMetaData("EntityAbstract", ECClassType::Entity, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithJustDomain", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("EntitySealed", ECClassType::Struct, ECClassModifier::Sealed));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithAllFlags", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithDomainAndCa", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("EntityWithDomainAndStruct", ECClassType::Struct, ECClassModifier::None));

    expectedClasses.push_back(ExpectedClassMetaData("StructAbstract", ECClassType::Entity, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithJustStruct", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("StructSealed", ECClassType::Struct, ECClassModifier::Sealed));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithAllFlags", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithStructAndCa", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("StructWithStructAndDomain", ECClassType::Struct, ECClassModifier::None));

    expectedClasses.push_back(ExpectedClassMetaData("CaAbstract", ECClassType::Entity, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithJustCa", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CaSealed", ECClassType::Struct, ECClassModifier::Sealed));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithAllFlags", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithCaAndDomain", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CaWithCaAndStruct", ECClassType::Struct, ECClassModifier::None));

    expectedClasses.push_back(ExpectedClassMetaData("RelationshipAbstract", ECClassType::Relationship, ECClassModifier::Abstract));
    expectedClasses.push_back(ExpectedClassMetaData("RelationshipConcrete", ECClassType::Relationship, ECClassModifier::None));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_Flat_ConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddConversionSchemaPath(ECTestFixture::GetTestDataPath(L"V3Conversion").c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchemaFlat.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load BadSchemaFlat for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    CreateExpectedWithAttributes(expectedClasses);

    bvector<Utf8CP> unexpectedClasses;
    VerifySchema(schema, expectedClasses, unexpectedClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_Flat_NoConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchemaFlat2.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load BadSchemaFlat2 for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    CreateExpectedWithOutAttributes(expectedClasses);

    bvector<Utf8CP> unexpectedClasses;
    VerifySchema(schema, expectedClasses, unexpectedClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_ConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddConversionSchemaPath(ECTestFixture::GetTestDataPath(L"V3Conversion").c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load BadSchema for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    CreateExpectedWithAttributes(expectedClasses);

    bvector<Utf8CP> unexpectedClasses;
    VerifySchema(schema, expectedClasses, unexpectedClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVersionConversionTests, CanLoadMetaSchemaWithDeliveredConversionSchema)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"MetaSchema.02.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load MetaSchema.02.00 for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    expectedClasses.push_back(ExpectedClassMetaData("PropertyCustomAttribute", ECClassType::CustomAttribute, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECClassDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECPropertyDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ConstraintClassDef", ECClassType::Struct, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("CustomAttributeDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECRelationshipClassDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECRelationshipConstraintDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECRelationshipSourceDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECRelationshipTargetDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSchemaDef", ECClassType::Entity, ECClassModifier::None));

    bvector<Utf8CP> unexpectedClasses;
    unexpectedClasses.push_back("CustomAttributeContainerHasCustomAttribute");
    unexpectedClasses.push_back("ECSupplementalClassDef");
    unexpectedClasses.push_back("ECSupplementalPropertyDef");
    unexpectedClasses.push_back("ECSupplementalRelationshipClassDef");
    unexpectedClasses.push_back("ECSupplementalRelationshipConstraintDef");
    unexpectedClasses.push_back("ECSupplementalRelationshipSourceDef");
    unexpectedClasses.push_back("ECSupplementalRelationshipTargetDef");
    unexpectedClasses.push_back("ECSupplementalSchemaDef");
    unexpectedClasses.push_back("SupplementalCustomAttributeDef");

    VerifySchema(schema, expectedClasses, unexpectedClasses);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
