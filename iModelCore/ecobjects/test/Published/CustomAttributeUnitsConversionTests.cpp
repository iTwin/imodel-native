/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeUnitsConversionTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <Units/Units.h> // Maybe need??

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitSpecificationConversionTest : ECTestFixture {};

struct UnitInstanceConversionTest : ECTestFixture
    {
    struct TestUnitResolver : ECInstanceReadContext::IUnitResolver
        {
        explicit TestUnitResolver(){}
        ~TestUnitResolver() {}
        Utf8String _ResolveUnitName(ECPropertyCR ecProperty) const override;
        };

    mutable TestUnitResolver m_testUnitResolver;

    virtual void SetUp() override
        {
        ECTestFixture::SetUp();
        }
    };

void validateUnitsInConvertedSchema(ECSchemaR convertedSchema, ECSchemaR originalSchema)
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

                EXPECT_EQ(0, strcmp(convertedUnit->GetName(), koq->GetPersistenceUnit().GetUnit()->GetName())) 
                    << "Converted unit not correct for " << convertedProp->GetName().c_str() << " Expected: " << convertedUnit->GetName() << " Actual: " << koq->GetPersistenceUnit().GetUnit()->GetName();
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                        Caleb.Shafer            06/2017
// Test that references are properly removed when there is no schema level 'UnitSpecifications' CA, only property level ones
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, SchemaWithOldUnitSpecification_OnlyOnProperty)
    {
    Utf8String schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="OldUnits" version="01.00" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
        <ECClass typeName="TestClass" isDomainClass="True">
            <ECProperty propertyName="Length" typeName="double">
                <ECCustomAttributes>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <KindOfQuantityName>LENGTH</KindOfQuantityName>
                        <DimensionName>L</DimensionName>
                        <UnitName>FOOT</UnitName>
                        <AllowableUnits />
                    </UnitSpecification>
                </ECCustomAttributes>
            </ECProperty>
        </ECClass>
    </ECSchema>)xml";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext)) << "Failed to load schema with old unit";
    ECSchemaPtr originalSchema;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CopySchema(originalSchema)) << "Failed to copy schema";

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema)) << "Failed to convert schema";
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    ASSERT_EQ(0, schema->GetReferencedSchemas().size()) << "Expected no schema references after conversion because the only reference in the original schema was the Unit_Attributes schema";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                        Caleb.Shafer            06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, SchemaWithOldUnitSpecifications)
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
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    ASSERT_EQ(0, schema->GetReferencedSchemas().size()) << "Expected no schema references after conversion because the only reference in the original schema was the Unit_Attributes schema";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, BaseAndDerivedUnitsAreNotCompatible)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="SpecialPipe" displayLabel="A more specialized pipe" isDomainClass="True">
                <BaseClass>Pipe</BaseClass>
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>AREA</KindOfQuantityName>
                            <DimensionName>L2</DimensionName>
                            <UnitName>ACRE</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>DECIMETRE</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>

        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    ASSERT_FALSE(ECSchemaConverter::Convert(*schema.get())) << "Converted a schema with incompatible units";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, PersistenceUnitChange)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="SpecialPipe" displayLabel="A more specialized pipe" isDomainClass="True">
                <BaseClass>Pipe</BaseClass>
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>FOOT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>DECIMETRE</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>

        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Failed to convert schema";

    ECClassCP pipe;
    ECClassCP specialPipe;
    ECPropertyP pipeLength;
    ECPropertyP specialPipeLength;
    KindOfQuantityCP lengthKOQ;
    KindOfQuantityCP specialLengthKOQ;

    pipe = schema->GetClassCP("Pipe");
    EXPECT_NE(pipe, nullptr);

    pipeLength = pipe->GetPropertyP("Length");
    EXPECT_NE(pipeLength, nullptr);
    EXPECT_TRUE(pipeLength->IsKindOfQuantityDefinedLocally());
    
    lengthKOQ = pipeLength->GetKindOfQuantity();

    specialPipe = schema->GetClassCP("SpecialPipe");
    EXPECT_NE(specialPipe, nullptr);

    specialPipeLength = specialPipe->GetPropertyP("Length");
    EXPECT_NE(specialPipeLength, nullptr);
    EXPECT_TRUE(specialPipeLength->IsKindOfQuantityDefinedLocally());

    specialLengthKOQ = specialPipeLength->GetKindOfQuantity();

    EXPECT_EQ(2, schema->GetKindOfQuantityCount());
    EXPECT_NE(lengthKOQ, specialLengthKOQ); 

    EXPECT_STREQ(lengthKOQ->GetPersistenceUnit().GetUnit()->GetName(), specialLengthKOQ->GetPersistenceUnit().GetUnit()->GetName());
    EXPECT_STRNE(lengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName(), specialLengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName());
    EXPECT_STREQ("DM", lengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName());
    EXPECT_STREQ("FT", specialLengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName());

    auto oldPersistenceUnit = specialPipeLength->GetCustomAttributeLocal("OldPersistenceUnit");
    ECValue oldUnitName;
    oldPersistenceUnit->GetValue(oldUnitName, "Name");
    EXPECT_STREQ("FOOT", oldUnitName.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                        10/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, SameKOQMultiplePersistenceUnits)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>DECIMETRE</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>

            <ECClass typeName="SpecialPipe" displayLabel="A more specialized pipe" isDomainClass="True">
                <BaseClass>Pipe</BaseClass>
                <ECProperty propertyName="AnotherLength" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>FOOT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>FOOT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="YetAnotherLength" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>FOOT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Failed to convert schema";
    ASSERT_STREQ("DM", schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetPersistenceUnit().GetUnit()->GetName());
    ASSERT_FALSE(schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    ASSERT_STREQ("DM", schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetPersistenceUnit().GetUnit()->GetName());
    ASSERT_STREQ("FT", schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetDefaultPresentationUnit().GetUnit()->GetName());
    ASSERT_TRUE(schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    ASSERT_STREQ("FT", schema->GetClassCP("SpecialPipe")->GetPropertyP("AnotherLength")->GetKindOfQuantity()->GetPersistenceUnit().GetUnit()->GetName());
    ASSERT_FALSE(schema->GetClassCP("SpecialPipe")->GetPropertyP("AnotherLength")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    ASSERT_STREQ("FT", schema->GetClassCP("SpecialPipe")->GetPropertyP("YetAnotherLength")->GetKindOfQuantity()->GetPersistenceUnit().GetUnit()->GetName());
    ASSERT_FALSE(schema->GetClassCP("SpecialPipe")->GetPropertyP("YetAnotherLength")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    ASSERT_NE(nullptr, schema->GetKindOfQuantityCP("LENGTH"));
    ASSERT_NE(nullptr, schema->GetKindOfQuantityCP("LENGTH_SpecialPipe"));
    ASSERT_NE(nullptr, schema->GetKindOfQuantityCP("LENGTH_SPecialPipe_Length"));
    ASSERT_EQ(3, schema->GetKindOfQuantityCount());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, PersistenceUnitChange_WithPresentationUnits)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>DECIMETRE</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayUnitName>KILOMETRE</DisplayUnitName>
                        </DisplayUnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>

            <ECClass typeName="SpecialPipe" displayLabel="A more specialized pipe" isDomainClass="True">
                <BaseClass>Pipe</BaseClass>
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>FOOT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayUnitName>CENTIMETRE</DisplayUnitName>
                        </DisplayUnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Failed to convert schema";

    ECClassCP pipe;
    ECClassCP specialPipe;
    ECPropertyP pipeLength;
    ECPropertyP specialPipeLength;
    KindOfQuantityCP lengthKOQ;
    KindOfQuantityCP specialLengthKOQ;

    pipe = schema->GetClassCP("Pipe");
    EXPECT_NE(pipe, nullptr);

    pipeLength = pipe->GetPropertyP("Length");
    EXPECT_NE(pipeLength, nullptr);
    EXPECT_TRUE(pipeLength->IsKindOfQuantityDefinedLocally());
    
    lengthKOQ = pipeLength->GetKindOfQuantity();

    specialPipe = schema->GetClassCP("SpecialPipe");
    EXPECT_NE(specialPipe, nullptr);

    specialPipeLength = specialPipe->GetPropertyP("Length");
    EXPECT_NE(specialPipeLength, nullptr);
    EXPECT_TRUE(specialPipeLength->IsKindOfQuantityDefinedLocally());

    specialLengthKOQ = specialPipeLength->GetKindOfQuantity();

    EXPECT_EQ(2, schema->GetKindOfQuantityCount());
    EXPECT_NE(lengthKOQ, specialLengthKOQ); 

    EXPECT_STREQ(lengthKOQ->GetPersistenceUnit().GetUnit()->GetName(), specialLengthKOQ->GetPersistenceUnit().GetUnit()->GetName());
    EXPECT_STRNE(lengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName(), specialLengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName());
    EXPECT_STREQ("KM", lengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName());
    EXPECT_STREQ("CM", specialLengthKOQ->GetDefaultPresentationUnit().GetUnit()->GetName());
    EXPECT_EQ(1, lengthKOQ->GetPresentationUnitList().size());
    EXPECT_EQ(1, specialLengthKOQ->GetPresentationUnitList().size());

    auto oldPersistenceUnit = specialPipeLength->GetCustomAttributeLocal("OldPersistenceUnit");
    ECValue oldUnitName;
    oldPersistenceUnit->GetValue(oldUnitName, "Name");
    EXPECT_STREQ("FOOT", oldUnitName.GetUtf8CP());
    }

//=======================================================================================
//! UnitInstanceConversionTest
//=======================================================================================

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitInstanceConversionTest::TestUnitResolver::_ResolveUnitName(ECPropertyCR ecProperty) const
    {
    if (!ecProperty.IsDefinedLocal("ECv3ConversionAttributes", "OldPersistenceUnit"))
        return "";

    IECInstancePtr instance = ecProperty.GetCustomAttribute("ECv3ConversionAttributes", "OldPersistenceUnit");
    ECValue unitName;
    instance->GetValue(unitName, "Name");

    if (unitName.IsNull() || !unitName.IsUtf8())
        return "";

    return unitName.GetUtf8CP();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitInstanceConversionTest, BasicTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>DECIMETRE</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>

            <ECClass typeName="SpecialPipe" displayLabel="A more specialized pipe" isDomainClass="True">
                <BaseClass>Pipe</BaseClass>
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>LENGTH</KindOfQuantityName>
                            <DimensionName>L</DimensionName>
                            <UnitName>FOOT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get())) << "Failed to convert schema";

    {
    Utf8CP instanceXml = R"xml(
        <SpecialPipe xmlns="OldUnits.01.00">
            <Length>50</Length>
        </SpecialPipe>
        )xml";

    IECInstancePtr testInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);
    instanceContext->SetUnitResolver(&m_testUnitResolver);
    InstanceReadStatus readStat = IECInstance::ReadFromXmlString(testInstance, instanceXml, *instanceContext);
    ASSERT_EQ(InstanceReadStatus::Success, readStat);

    ECValue length;
    testInstance->GetValue(length, "Length");
    EXPECT_EQ(50*3.048, length.GetDouble());
    }
    {
    Utf8CP instanceXml = R"xml(
        <SpecialPipe xmlns="OldUnits.01.00">
            <Length>50</Length>
        </SpecialPipe>
        )xml";

    IECInstancePtr testInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);
    InstanceReadStatus readStat = IECInstance::ReadFromXmlString(testInstance, instanceXml, *instanceContext);
    ASSERT_EQ(InstanceReadStatus::Success, readStat);

    ECValue length;
    testInstance->GetValue(length, "Length");
    EXPECT_EQ(50, length.GetDouble());
    }

    }

END_BENTLEY_ECN_TEST_NAMESPACE
