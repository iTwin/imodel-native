/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaVersionConverstionTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <Units\Units.h>

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

TEST_F(SchemaVersionConversionTests, SchemaWithBadFlags_NoConversionSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"BadSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Expected BadSchema to fail to load without conversion schema";
    }

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
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalClassDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalPropertyDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalRelationshipClassDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalRelationshipConstraintDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalRelationshipSourceDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalRelationshipTargetDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("ECSupplementalSchemaDef", ECClassType::Entity, ECClassModifier::None));
    expectedClasses.push_back(ExpectedClassMetaData("SupplementalCustomAttributeDef", ECClassType::Entity, ECClassModifier::None));

    bvector<Utf8CP> unexpectedClasses;
    unexpectedClasses.push_back("CustomAttributeContainerHasCustomAttribute");
    unexpectedClasses.push_back("CustomAttributeContainerHasLocalCustomAttribute");

    VerifySchema(schema, expectedClasses, unexpectedClasses);
    }

void ValidateUnitsInConvertedSchema(ECSchemaR convertedSchema, ECSchemaR originalSchema)
    {
    for (const auto& ecClass : originalSchema.GetClasses())
        {
        ECClassP convertedClass = convertedSchema.GetClassP(ecClass->GetName().c_str());
        for (const auto& ecProp : ecClass->GetProperties(true))
            {
            Unit originalUnit;
            if (Unit::GetUnitForECProperty(originalUnit, *ecProp))
                {
                ECPropertyP convertedProp = convertedClass->GetPropertyP(ecProp->GetName().c_str());
                KindOfQuantityCP koq = convertedProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                ASSERT_NE(nullptr, koq) << "Could not find KOQ for property " << ecClass->GetName().c_str() << ":" << ecProp->GetName().c_str();
                Units::UnitCP convertedUnit = Units::UnitRegistry::Instance().LookupUnitUsingOldName(originalUnit.GetName());
                if (nullptr == convertedUnit) // If null it may be a dummy unit added during conversion ... 
                    convertedUnit = Units::UnitRegistry::Instance().LookupUnit(originalUnit.GetName());
                ASSERT_NE(nullptr, convertedUnit) << "Could not find converted unit for old unit " << originalUnit.GetName();

                EXPECT_EQ(0, strcmp(convertedUnit->GetName(), koq->GetPersistenceUnit().c_str())) << "Converted unit not correct for " << convertedProp->GetName().c_str();
                }
            }
        }
    }

TEST_F(SchemaVersionConversionTests, SchemaWithOldUnitSpecifications)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    WString testSchemaPath = ECTestFixture::GetTestDataPath(L"OldUnits.01.00.ecschema.xml");

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, testSchemaPath.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Failed to convert schema";
    
    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    ECSchemaPtr originalSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(originalSchema, testSchemaPath.c_str(), *context2));
    ValidateUnitsInConvertedSchema(*schema, *originalSchema);
    }

//TEST_F(SchemaVersionConversionTests, OpenPlantSchema)
//    {
//    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
//
//    WString testSchemaPath = L"C:\\Users\\Colin.Kerr.BENTLEY\\Documents\\Schemas\\OPPID 107 4-5-2016\\OP Schemas";
//    schemaContext->AddSchemaPath(testSchemaPath.c_str());
//
//    SchemaKey key("OpenPlant", 1, 0, 7);
//    ECSchemaPtr schema = schemaContext->LocateSchema(key, SchemaMatchType::Exact);
//    ASSERT_TRUE(schema.IsValid());
//
//    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Failed to convert schema";
//
//    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
//    context2->AddSchemaPath(testSchemaPath.c_str());
//    ECSchemaPtr originalSchema = context2->LocateSchema(key, SchemaMatchType::Exact);
//    ASSERT_TRUE(originalSchema.IsValid());
//    schema->WriteToXmlFile(L"C:\\diff\\OpenPlant.01.00.07.ecschema.xml", 3);
//    ValidateUnitsInConvertedSchema(*schema, *originalSchema);
//    }
END_BENTLEY_ECN_TEST_NAMESPACE
