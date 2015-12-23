/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaVersionConverstionTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

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

void VerifySchema(ECSchemaPtr schema, bvector<ExpectedClassMetaData> expectedClasses)
    {
    for (auto const& expected : expectedClasses)
        {
        ECClassCP expectedClass = schema->GetClassCP(expected.ClassName);
        ASSERT_NE(nullptr, expectedClass) << "Could not find class '" << expected.ClassName << "' in converted schema";
        ASSERT_EQ(expected.Type, expectedClass->GetClassType()) << "Type of class incorrect for '" << expected.ClassName << "' in converted schema";
        ASSERT_EQ(expected.Modifier, expectedClass->GetClassModifier()) << "Modifer for class incorrect for '" << expected.ClassName << "' in converted schema";
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

TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_Flat_ConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddConversionSchemaPath(ECTestFixture::GetTestDataPath(L"V3Conversion").c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchemaFlat.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load BadSchemaFlat for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    CreateExpectedWithAttributes(expectedClasses);

    VerifySchema(schema, expectedClasses);
    }

TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_Flat_NoConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchemaFlat2.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load BadSchemaFlat2 for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    CreateExpectedWithOutAttributes(expectedClasses);

    VerifySchema(schema, expectedClasses);
    }

TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_ConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddConversionSchemaPath(ECTestFixture::GetTestDataPath(L"V3Conversion").c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load BadSchema for test";

    bvector<ExpectedClassMetaData> expectedClasses;
    CreateExpectedWithAttributes(expectedClasses);

    VerifySchema(schema, expectedClasses);
    }

TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_NoConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Expected BadSchema to fail to load without conversion schema";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
