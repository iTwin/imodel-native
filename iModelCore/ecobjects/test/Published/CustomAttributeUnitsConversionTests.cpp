/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/CustomAttributeUnitsConversionTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <Units/Units.h> // Maybe need??

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitSpecificationConversionTest : ECTestFixture {};

struct UnitsCustomAttributesConversionTests : ECTestFixture
    {
    ECSchemaPtr     m_schema;
    void CreateTestSchema();
    void SerializeAndCheck(ECSchemaPtr &outputSchema, ECSchemaPtr inputSchema, bvector<Utf8CP> customAttributeNames, bvector<Utf8CP> customAttributeNamesInMemory);

    //---------------------------------------------------------------------------------------//
    // Stores the format of the reference schema xml as a string
    // @bsimethod                             Prasanna.Prakash                       03/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    static Utf8CP   TestSchemaXmlString()
        {
        Utf8CP format = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
            "  <ECSchemaReference name=\"Unit_Attributes\" version=\"01.00.00\" prefix=\"units_attribs\" />"
            "  <ECCustomAttributes>"
            "      <IsUnitSystemSchema xmlns=\"Unit_Attributes.01.00\" />"
            "      <UnitSpecifications xmlns=\"Unit_Attributes.01.00\">"
            "          <UnitSpecificationList>"
            "              <UnitSpecification>"
            "                  <KindOfQuantityName>LENGTH</KindOfQuantityName>"
            "                  <UnitName>KILOMETRE</UnitName>"
            "              </UnitSpecification>"
            "              <UnitSpecification>"
            "                  <DimensionName>L</DimensionName>"
            "                  <UnitName>KILOMETRE</UnitName>"
            "              </UnitSpecification>"
            "          </UnitSpecificationList>"
            "      </UnitSpecifications>"
            "  </ECCustomAttributes>"
            "  <ECEntityClass typeName =\"TestClass\">"
            "        <ECProperty propertyName=\"PropertyA\" typeName=\"double\">"
            "          <ECCustomAttributes>"
            "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
            "                  <UnitName>KILOMETRE</UnitName>"
            "              </UnitSpecification>"
            "          </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName=\"PropertyB\" typeName=\"double\">"
            "          <ECCustomAttributes>"
            "              <DisplayUnitSpecification xmlns = \"Unit_Attributes.01.00\">"
            "                  <DisplayUnitName>MILE</DisplayUnitName>"
            "                  <DisplayFormatString>0000.###### \"ignored\"</DisplayFormatString>"
            "              </DisplayUnitSpecification>"
            "          </ECCustomAttributes>"
            "        </ECProperty>"
            "    </ECEntityClass>"
            "</ECSchema>";

        return format;
        }

    };

struct UnitInstanceConversionTest : ECTestFixture
    {
    struct TestUnitResolver : ECInstanceReadContext::IUnitResolver
        {
        explicit TestUnitResolver(){}
        ~TestUnitResolver() {}
        Utf8String _ResolveUnitName(ECPropertyCR ecProperty) const override;
        };

    mutable TestUnitResolver m_testUnitResolver;
    };

void verifyReferencedSchemas(ECSchemaR convertedSchema, bvector<Utf8String> expectedReferenceFullNames)
    {
    EXPECT_EQ(expectedReferenceFullNames.size(), convertedSchema.GetReferencedSchemas().size());

    for (auto const& schemaReference : convertedSchema.GetReferencedSchemas())
        {
        Utf8String refSchemaFullName = schemaReference.first.GetFullSchemaName();
        auto it = std::find(expectedReferenceFullNames.begin(), expectedReferenceFullNames.end(), refSchemaFullName);
        EXPECT_NE(expectedReferenceFullNames.end(), it) << "Found unexpected schema reference: " << refSchemaFullName.c_str();
        if (expectedReferenceFullNames.end() != it)
            expectedReferenceFullNames.erase(it);
        }
    if (0 != expectedReferenceFullNames.size())
        {
        Utf8String referencesNotFound = BeStringUtilities::Join(expectedReferenceFullNames, ", ");
        EXPECT_EQ(0, expectedReferenceFullNames.size()) << "Did not find expected reference schemas: " << referencesNotFound.c_str();
        }
    }

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
                KindOfQuantityCP koq = convertedProp->GetKindOfQuantity();
                ASSERT_NE(nullptr, koq) << "Could not find KOQ for property " << ecClass->GetName().c_str() << ":" << ecProp->GetName().c_str();

                Utf8CP ecName = Units::UnitNameMappings::TryGetECNameFromOldName(originalUnit.GetName());
                ASSERT_NE(nullptr, ecName) << "Mapping for original unit '" << originalUnit.GetName() << "' to ECName does not exist.";

                Utf8String alias;
                Utf8String unitName;
                ECClass::ParseClassName(alias, unitName, ecName);

                ECUnitCP originalUnitInNewSystem = ECTestFixture::GetUnitsSchema()->GetUnitCP(unitName.c_str());
                ASSERT_NE(nullptr, originalUnitInNewSystem) << "Could not find converted unit for old unit " << originalUnit.GetName();

                bool unitShouldBeConvertedToSI = !originalUnitInNewSystem->IsSI();
                Units::UnitCP convertedUnit = unitShouldBeConvertedToSI ? originalUnitInNewSystem->GetPhenomenon()->GetSIUnit() : originalUnitInNewSystem;
                ASSERT_NE(nullptr, convertedUnit) << "Could not find SI unit for original unit " << originalUnitInNewSystem->GetName();

                if (unitShouldBeConvertedToSI)
                    {
                    auto oldUnitCA = convertedProp->GetCustomAttribute("ECv3ConversionAttributes", "OldPersistenceUnit");
                    ASSERT_TRUE(oldUnitCA.IsValid()) << convertedClass->GetName().c_str() << "." << convertedProp->GetName().c_str() 
                                                        << " the unit should have been converted to SI so expected the OldPersistenceUnit CA to be applied to the property";
                    ECValue oldName;
                    ASSERT_EQ(ECObjectsStatus::Success, oldUnitCA->GetValue(oldName, "Name"));
                    ASSERT_STREQ(originalUnit.GetName(), oldName.GetUtf8CP()) << "Old unit name not persisted correctly";
                    }

                EXPECT_EQ(0, strcmp(convertedUnit->GetName().c_str(), koq->GetPersistenceUnit()->GetName().c_str())) 
                    << "Converted unit not correct for " << convertedProp->GetName().c_str() << " Expected: " << convertedUnit->GetName() << " Actual: " << koq->GetPersistenceUnit()->GetName();
                }
            }
        }
    }

//=======================================================================================
//! UnitSpecificationConversionTest
//=======================================================================================

//---------------------------------------------------------------------------------------
//@bsimethod                                        Caleb.Shafer            06/2017
// Test that references are properly removed when there is no schema level 'UnitSpecifications' CA, only property level ones
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, SchemaWithOldUnitSpecification_OnArrayProperty)
    {
    Utf8String schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="OldUnits" version="01.00" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
        <ECClass typeName="TestClass" isDomainClass="True">
            <ECArrayProperty propertyName="Length" typeName="double">
                <ECCustomAttributes>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <KindOfQuantityName>LENGTH</KindOfQuantityName>
                        <DimensionName>L</DimensionName>
                        <UnitName>FOOT</UnitName>
                        <AllowableUnits />
                    </UnitSpecification>
                </ECCustomAttributes>
            </ECArrayProperty>
        </ECClass>
    </ECSchema>)xml";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext)) << "Failed to load schema with old unit";
    ECSchemaPtr originalSchema;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CopySchema(originalSchema)) << "Failed to copy schema";

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, schemaContext.get())) << "Failed to convert schema";
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("ECv3ConversionAttributes.01.00.00");
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, schemaContext.get())) << "Failed to convert schema";
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("ECv3ConversionAttributes.01.00.00");
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                        Colin.Kerr              11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, UnitSpecificationWithUnknownUnitName)
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
                        <UnitName>SILLYMETER</UnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, schemaContext.get())) << "Failed to convert schema";
    
    ASSERT_EQ(0, schema->GetKindOfQuantityCount()) << "No KOQ should have been created when unit is unknown";
    ASSERT_EQ(nullptr, schema->GetClassCP("TestClass")->GetPropertyP("Length")->GetKindOfQuantity()) << "No KOQ should have been added to the property when the unit is unknown";

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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), schemaContext.get())) << "Failed to convert schema";
    
    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    ECSchemaPtr originalSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(originalSchema, testSchemaPath.c_str(), *context2));
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("ECv3ConversionAttributes.01.00.00");
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, PersistenceAndPresentationUnitsNotCompatibleDropsPresentationUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="Length" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <DimensionName>M_PER_L3</DimensionName>
                            <KindOfQuantityName>DENSITY</KindOfQuantityName>
                            <UnitName>KILOGRAM_PER_METRE_CUBED</UnitName>
                        </UnitSpecification>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayFormatString>F4</DisplayFormatString>
                            <DisplayUnitName>METRE_SQUARED</DisplayUnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

    auto koq = schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetKindOfQuantity();
    EXPECT_STREQ("KG_PER_CUB_M", koq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_FALSE(koq->HasPresentationFormats());

    ASSERT_EQ(1, schema->GetReferencedSchemas().size()) << "Expected a single schema references after conversion because the standard Units schema ia added";

    ASSERT_TRUE(ECSchema::IsSchemaReferenced(*schema, *ECTestFixture::GetUnitsSchema()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                 06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, BaseAndDerivedUnitsNotCompatibleFailsConversion)
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

    ASSERT_FALSE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Converted a schema with incompatible units";
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

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

    EXPECT_STREQ(lengthKOQ->GetPersistenceUnit()->GetName().c_str(), specialLengthKOQ->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STRNE(lengthKOQ->GetDefaultPresentationFormat()->GetName().c_str(), specialLengthKOQ->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:DM]", lengthKOQ->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:FT]", specialLengthKOQ->GetDefaultPresentationFormat()->GetName().c_str());

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
    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";
    EXPECT_STREQ("M", schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:DM]", schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("LENGTH", schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_TRUE(schema->GetClassCP("Pipe")->GetPropertyP("Length")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    EXPECT_STREQ("M", schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:FT]", schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("LENGTH_SpecialPipe", schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_TRUE(schema->GetClassCP("SpecialPipe")->GetPropertyP("Length")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    EXPECT_STREQ("M", schema->GetClassCP("SpecialPipe")->GetPropertyP("AnotherLength")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:FT]", schema->GetClassCP("SpecialPipe")->GetPropertyP("AnotherLength")->GetKindOfQuantity()->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("LENGTH_SpecialPipe", schema->GetClassCP("SpecialPipe")->GetPropertyP("AnotherLength")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_TRUE(schema->GetClassCP("SpecialPipe")->GetPropertyP("AnotherLength")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    EXPECT_STREQ("M", schema->GetClassCP("SpecialPipe")->GetPropertyP("YetAnotherLength")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:FT]", schema->GetClassCP("SpecialPipe")->GetPropertyP("YetAnotherLength")->GetKindOfQuantity()->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("LENGTH_SpecialPipe", schema->GetClassCP("SpecialPipe")->GetPropertyP("YetAnotherLength")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_TRUE(schema->GetClassCP("SpecialPipe")->GetPropertyP("YetAnotherLength")->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldPersistenceUnit").IsValid());
    EXPECT_EQ(2, schema->GetKindOfQuantityCount());
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

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

    EXPECT_STREQ(lengthKOQ->GetPersistenceUnit()->GetName().c_str(), specialLengthKOQ->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STRNE(lengthKOQ->GetDefaultPresentationFormat()->GetName().c_str(), specialLengthKOQ->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:KM]", lengthKOQ->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STREQ("DefaultRealU[u:CM]", specialLengthKOQ->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_EQ(1, lengthKOQ->GetPresentationFormats().size());
    EXPECT_EQ(1, specialLengthKOQ->GetPresentationFormats().size());

    auto oldPersistenceUnit = specialPipeLength->GetCustomAttributeLocal("OldPersistenceUnit");
    ECValue oldUnitName;
    oldPersistenceUnit->GetValue(oldUnitName, "Name");
    EXPECT_STREQ("FOOT", oldUnitName.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                 01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, PercentUnitsPassThroughWithoutChangeToSI)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="Ratios" isDomainClass="True">
                <ECProperty propertyName="Percent" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>PERCENTAGES</KindOfQuantityName>
                            <DimensionName>ONE</DimensionName>
                            <UnitName>PERCENT_PERCENT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="OtherPercent" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>OTHER_PERCENTAGES</KindOfQuantityName>
                            <DimensionName>ONE</DimensionName>
                            <UnitName>UNITLESS_PERCENT</UnitName>
                            <AllowableUnits />
                        </UnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="DerivedRatios" isDomainClass="True">
                <BaseClass>Ratios</BaseClass>
                <ECProperty propertyName="Percent" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <KindOfQuantityName>PERCENTAGES</KindOfQuantityName>
                            <DimensionName>ONE</DimensionName>
                            <UnitName>UNITLESS_PERCENT</UnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

    EXPECT_STREQ("PERCENTAGES", schema->GetClassCP("Ratios")->GetPropertyP("Percent")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_STREQ("PERCENT", schema->GetClassCP("Ratios")->GetPropertyP("Percent")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("OTHER_PERCENTAGES", schema->GetClassCP("Ratios")->GetPropertyP("OtherPercent")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_STREQ("DECIMAL_PERCENT", schema->GetClassCP("Ratios")->GetPropertyP("OtherPercent")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("PERCENTAGES_DerivedRatios", schema->GetClassCP("DerivedRatios")->GetPropertyP("Percent")->GetKindOfQuantity()->GetName().c_str());
    EXPECT_STREQ("DECIMAL_PERCENT", schema->GetClassCP("DerivedRatios")->GetPropertyP("OtherPercent")->GetKindOfQuantity()->GetPersistenceUnit()->GetName().c_str());
    ASSERT_TRUE(schema->GetClassCP("DerivedRatios")->GetPropertyP("Percent")->GetCustomAttributeLocal("OldPersistenceUnit").IsValid());
    ECValue oldUnitName;
    ASSERT_EQ(ECObjectsStatus::Success, schema->GetClassCP("DerivedRatios")->GetPropertyP("Percent")->GetCustomAttributeLocal("OldPersistenceUnit")->GetValue(oldUnitName, "Name"));
    EXPECT_STREQ("UNITLESS_PERCENT", oldUnitName.GetUtf8CP());
    }

//=======================================================================================
//! UnitsCustomAttributesConversionTests
//=======================================================================================

//---------------------------------------------------------------------------------------//
// Creates the test schema using the TestSchemaXml string
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
void UnitsCustomAttributesConversionTests::CreateTestSchema()
    {
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, TestSchemaXmlString(), *schemaContext))
            << "Failed to read the test reference schema from xml string";
    ASSERT_TRUE(m_schema.IsValid()) << "Test Schema is not valid";
    }

//---------------------------------------------------------------------------------------//
// Serializes the schema, checks for the presence/absence of strings in the serialized xml 
// and that the schema can be loaded again
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
void UnitsCustomAttributesConversionTests::SerializeAndCheck(ECSchemaPtr &outputSchema, ECSchemaPtr inputSchema, bvector<Utf8CP> customAttributeNames, bvector<Utf8CP> customAttributeNamesInMemory)
    {
    Utf8String schemaXmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, inputSchema->WriteToXmlString(schemaXmlString, ECVersion::V3_0))
           << "Cannot serialize the schema";

    for (auto customAttributeName : customAttributeNames)
        ASSERT_NE(Utf8String::npos, schemaXmlString.find(customAttributeName));
    for (auto inMemoryName : customAttributeNamesInMemory)
        ASSERT_EQ(Utf8String::npos, schemaXmlString.find(inMemoryName));

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(outputSchema, schemaXmlString.c_str(), *schemaContext))
           << "Failed to read the test reference schema from xml string";
    }

//---------------------------------------------------------------------------------------//
// Tests that Custom Attributes are not lost due to the  'Attr' name swizzling we do to 
// load CAs for classes which were marked as both CA and Struct in EC2 
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, TestUnitsCustomAttributesAreNotLostOnRoundTrip)
    {
    CreateTestSchema();

    ASSERT_TRUE(m_schema->IsDefined("Unit_Attributes", "IsUnitSystemSchema"))
             << "IsUnitSystemSchema custom attribute was not read at schema level";
    ASSERT_TRUE(m_schema->IsDefined("Unit_Attributes", "UnitSpecifications"))
        << "UnitSpecifications custom attribute was not read at schema level";
    ASSERT_TRUE(m_schema->GetClassP("TestClass")->GetPropertyP("PropertyA")->IsDefined("Unit_Attributes", "UnitSpecificationAttr"))
        << "UnitSpecifications custom attribute was not read at property level";
    ASSERT_TRUE(m_schema->GetClassP("TestClass")->GetPropertyP("PropertyB")->IsDefined("Unit_Attributes", "DisplayUnitSpecificationAttr"))
        << "UnitSpecifications custom attribute was not read at schema level";

    bvector<Utf8CP> stringsToFind;
    // CustomAttribute Names
    stringsToFind.push_back("IsUnitSystemSchema");
    stringsToFind.push_back("UnitSpecifications");
    stringsToFind.push_back("UnitSpecification");
    stringsToFind.push_back("DisplayUnitSpecification");
    // Property Names
    stringsToFind.push_back("UnitSpecificationList");
    stringsToFind.push_back("UnitName");
    stringsToFind.push_back("KindOfQuantityName");
    stringsToFind.push_back("DimensionName");
    stringsToFind.push_back("DisplayUnitName");
    stringsToFind.push_back("DisplayFormatString");

    bvector<Utf8CP> stringsToNotFind;
    stringsToNotFind.push_back("UnitSpecificationAttr");
    stringsToNotFind.push_back("DisplayUnitSpecificationAttr");
    ECSchemaPtr schema;
    SerializeAndCheck(schema, m_schema, stringsToFind, stringsToNotFind);

    ASSERT_TRUE(schema->IsDefined("Unit_Attributes", "IsUnitSystemSchema"))
             << "IsUnitSystemSchema custom attribute was not read at schema level";
    ASSERT_TRUE(schema->IsDefined("Unit_Attributes", "UnitSpecifications"))
             << "UnitSpecifications custom attribute was not read at schema level";
    ASSERT_TRUE(schema->GetClassP("TestClass")->GetPropertyP("PropertyA")->IsDefined("Unit_Attributes", "UnitSpecificationAttr"))
             << "UnitSpecifications custom attribute was not read at property level";
    ASSERT_TRUE(schema->GetClassP("TestClass")->GetPropertyP("PropertyB")->IsDefined("Unit_Attributes", "DisplayUnitSpecificationAttr"))
             << "UnitSpecifications custom attribute was not read at schema level";
    }

//---------------------------------------------------------------------------------------//
//* @bsimethod                                Colin.Kerr                       07/2017
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, OldUnitsWithKoqNameConflicts)
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
        <ECClass typeName="LENGTH" isDomainClass="True" />
    </ECSchema>)xml";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext)) << "Failed to load schema with old unit";
    ECSchemaPtr originalSchema;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CopySchema(originalSchema)) << "Failed to copy schema";

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, schemaContext.get())) << "Failed to convert schema";
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("ECv3ConversionAttributes.01.00.00");
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, DisplayUnitSpecificationIsRemovedWhenNoUnitSpecificationCanBeFound)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
            <ECClass typeName="Pipe" displayLabel="A generic pipe" isDomainClass="True">
                <ECProperty propertyName="ShouldDrop" typeName="double">
                    <ECCustomAttributes>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayFormatString>F4</DisplayFormatString>
                            <DisplayUnitName>METRE_SQUARED</DisplayUnitName>
                        </DisplayUnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="ShouldDrop2" typeName="double">
                    <ECCustomAttributes>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayFormatString>F4</DisplayFormatString>
                        </DisplayUnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="ShouldWork" typeName="double">
                    <ECCustomAttributes>
                        <UnitSpecification xmlns="Unit_Attributes.01.00">
                            <DimensionName>M_PER_L3</DimensionName>
                            <KindOfQuantityName>DENSITY</KindOfQuantityName>
                            <UnitName>KILOGRAM_PER_METRE_CUBED</UnitName>
                        </UnitSpecification>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayFormatString>F4</DisplayFormatString>
                            <DisplayUnitName>BANANA_APPLE_JUICE</DisplayUnitName>
                        </DisplayUnitSpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="SuperPipe" displayLabel="A super pipe" isDomainClass="True">
                <BaseClass>Pipe</BaseClass>
                <ECProperty propertyName="ShouldWork" typeName="double">
                    <ECCustomAttributes>
                        <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                            <DisplayFormatString>F4</DisplayFormatString>
                            <DisplayUnitName>POUND_PER_GALLON</DisplayUnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

    auto koq = schema->GetClassCP("Pipe")->GetPropertyP("ShouldWork")->GetKindOfQuantity();
    EXPECT_STREQ("KG_PER_CUB_M", koq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_FALSE(koq->HasPresentationFormats());
    EXPECT_EQ(nullptr, schema->GetClassCP("Pipe")->GetPropertyP("ShouldDrop")->GetKindOfQuantity());
    EXPECT_EQ(nullptr, schema->GetClassCP("Pipe")->GetPropertyP("ShouldDrop2")->GetKindOfQuantity());

    koq = schema->GetClassCP("SuperPipe")->GetPropertyP("ShouldWork")->GetKindOfQuantity();
    EXPECT_STREQ("KG_PER_CUB_M", koq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_TRUE(koq->HasPresentationFormats());

    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Joseph.Urbano                   08/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, DisplayUnitSpecificationUsesSameKOQNameAsUnitSpecification)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
        <ECClass typeName="PIPE" displayLabel="A generic pipe" isDomainClass="True">
            <ECProperty propertyName="PROPERTY1" typeName="double" >
                <ECCustomAttributes>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <DimensionName>L</DimensionName>
                        <KindOfQuantityName>DIAMETER</KindOfQuantityName>
                        <UnitName>FOOT</UnitName>
                    </UnitSpecification>
                    <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                        <DisplayFormatString>G6</DisplayFormatString>
                        <DisplayUnitName>FOOT</DisplayUnitName>
                    </DisplayUnitSpecification>
                </ECCustomAttributes>
            </ECProperty>
        </ECClass>
        <ECClass typeName="SUPER_PIPE" displayLabel="A super pipe" isDomainClass="True">
            <BaseClass>PIPE</BaseClass>
            <ECProperty propertyName="PROPERTY1" typeName="double">
                <ECCustomAttributes>
                    <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                        <DisplayFormatString>G6</DisplayFormatString>
                        <DisplayUnitName>INCH</DisplayUnitName>
                    </DisplayUnitSpecification>
                </ECCustomAttributes>
            </ECProperty>
            <ECProperty propertyName="PROPERTY2" typeName="double">
                <ECCustomAttributes>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <DimensionName>L</DimensionName>
                        <KindOfQuantityName>LENGTH</KindOfQuantityName>
                        <UnitName>FOOT</UnitName>
                    </UnitSpecification>
                    <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                        <DisplayFormatString>G6</DisplayFormatString>
                        <DisplayUnitName>INCH</DisplayUnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

    auto koq = schema->GetClassCP("PIPE")->GetPropertyP("PROPERTY1")->GetKindOfQuantity();
    EXPECT_STREQ("DIAMETER", koq->GetName().c_str());
    EXPECT_STREQ("M", koq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("FT", koq->GetDefaultPresentationFormat()->GetCompositeMajorUnit()->GetName().c_str());

    koq = schema->GetClassCP("SUPER_PIPE")->GetPropertyP("PROPERTY1")->GetKindOfQuantity();
    EXPECT_STREQ("DIAMETER_SUPER_PIPE", koq->GetName().c_str());
    EXPECT_STREQ("M", koq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("IN", koq->GetDefaultPresentationFormat()->GetCompositeMajorUnit()->GetName().c_str());

    koq = schema->GetClassCP("SUPER_PIPE")->GetPropertyP("PROPERTY2")->GetKindOfQuantity();
    EXPECT_STREQ("LENGTH", koq->GetName().c_str());
    EXPECT_STREQ("M", koq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("IN", koq->GetDefaultPresentationFormat()->GetCompositeMajorUnit()->GetName().c_str());

    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    expectedRefSchemas.push_back("ECv3ConversionAttributes.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Joseph.Urbano                   08/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, DisplayUnitSpecificationGetsKOQNameFromUnitSpecificationIfProcessedFirst)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
        <ECClass typeName="PIPE" displayLabel="A generic pipe" isDomainClass="True">
            <ECProperty propertyName="PROPERTY1" typeName="double" >
                <ECCustomAttributes>
                    <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                        <DisplayFormatString>G6</DisplayFormatString>
                        <DisplayUnitName>FOOT</DisplayUnitName>
                    </DisplayUnitSpecification>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <DimensionName>L</DimensionName>
                        <KindOfQuantityName>DIAMETER</KindOfQuantityName>
                        <UnitName>FOOT</UnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

    auto koq = schema->GetClassCP("PIPE")->GetPropertyP("PROPERTY1")->GetKindOfQuantity();
    EXPECT_STREQ("DIAMETER", koq->GetName().c_str());

    ASSERT_EQ(1, schema->GetKindOfQuantityCount()) << "The schema should only have one KindOfQuantity";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Joseph.Urbano                   08/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSpecificationConversionTest, KOQIsAcceptableEvenIfItHasNoPresentationFormats)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="OldUnits" version="01.00" displayLabel="Old Units test" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
        <ECClass typeName="BLOWER" displayLabel="A generic pipe" isDomainClass="True">
            <ECProperty propertyName="RATED_CURRENT" typeName="double" >
                <ECCustomAttributes>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <DimensionName>L</DimensionName>
                        <KindOfQuantityName>ELECTRIC_CURRENT</KindOfQuantityName>
                        <UnitName>AMPERE</UnitName>
                    </UnitSpecification>
                    <DisplayUnitSpecification xmlns="Unit_Attributes.01.00">
                        <DisplayFormatString>G6</DisplayFormatString>
                        <DisplayUnitName>AMPERE</DisplayUnitName>
                    </DisplayUnitSpecification>
                </ECCustomAttributes>
            </ECProperty>
        </ECClass>
        <ECClass typeName="MOTOR" displayLabel="A generic pipe" isDomainClass="True">
            <ECProperty propertyName="RATED_CURRENT" typeName="double" >
                <ECCustomAttributes>
                    <UnitSpecification xmlns="Unit_Attributes.01.00">
                        <DimensionName>L</DimensionName>
                        <KindOfQuantityName>ELECTRIC_CURRENT</KindOfQuantityName>
                        <UnitName>AMPERE</UnitName>
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

    auto koq = schema->GetClassCP("BLOWER")->GetPropertyP("RATED_CURRENT")->GetKindOfQuantity();
    EXPECT_STREQ("ELECTRIC_CURRENT", koq->GetName().c_str());

    koq = schema->GetClassCP("MOTOR")->GetPropertyP("RATED_CURRENT")->GetKindOfQuantity();
    EXPECT_STREQ("ELECTRIC_CURRENT", koq->GetName().c_str());

    ASSERT_EQ(1, schema->GetKindOfQuantityCount()) << "The schema should only have one KindOfQuantity";
    }

//---------------------------------------------------------------------------------------//
//* @bsimethod                                Colin.Kerr                       05/2017
//+---------------+---------------+---------------+---------------+---------------+------//
// Test that references are properly removed when there is no schema level 'UnitSpecifications' CA, only property level ones
TEST_F(UnitsCustomAttributesConversionTests, SchemaWithIsUnitSystemSchema_Attribute)
    {
    Utf8String schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="OldUnits" version="01.00" nameSpacePrefix="outs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECSchemaReference name="Unit_Attributes" version="01.00" prefix="units_attribs" />
        <IsUnitSystemSchema xmlns="Unit_Attributes.01.00" />
        <Mixed_UnitSystem xmlns="Unit_Attributes.01.00" />
        <SI_UnitSystem xmlns="Unit_Attributes.01.00" />
        <US_UnitSystem xmlns="Unit_Attributes.01.00" />
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

    ASSERT_TRUE(ECSchemaConverter::Convert(*schema, schemaContext.get())) << "Failed to convert schema";
    validateUnitsInConvertedSchema(*schema, *originalSchema);
    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("ECv3ConversionAttributes.01.00.00");
    expectedRefSchemas.push_back("Units.01.00.00");
    expectedRefSchemas.push_back("Formats.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    }


Utf8String getUnitName(ECSchemaCP schema, Utf8CP className, Utf8CP propertyName, Utf8CP caName, Utf8CP caPropName)
    {
    IECInstancePtr myUnitSpec = schema->GetClassCP(className)->GetPropertyP(propertyName)->GetCustomAttributeLocal(caName);
    if (myUnitSpec.IsValid())
        {
        ECValue unitName;
        myUnitSpec->GetValue(unitName, caPropName);
        return unitName.GetUtf8CP();
        }
    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Colin.Kerr                          01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsCustomAttributesConversionTests, EC3KOQsConvertBackToUnitSpecificationsCAs)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECSchemaReference name="ECv3ConversionAttributes" version="01.00" alias="V2ToV3" />
            <ECEntityClass typeName='A' modifier='abstract'>
                <ECProperty propertyName='PropA' typeName='double' kindOfQuantity='MyKindOfQuantity'>
                    <ECCustomAttributes>
                        <OldPersistenceUnit xmlns="ECv3ConversionAttributes.01.00">
                            <Name>FOOT</Name>
                        </OldPersistenceUnit>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECEntityClass typeName='B' modifer='none'>
                <BaseClass>A</BaseClass>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECEntityClass>
            <ECEntityClass typeName='C' modifer='sealed'>
                <BaseClass>B</BaseClass>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECEntityClass>
            <ECEntityClass typeName='D' modifer='sealed'>
                <ECProperty propertyName='Prop0' typeName='double' />
                <ECProperty propertyName='Prop1' typeName='double' kindOfQuantity='SecondKindOfQuantity' />
                <ECProperty propertyName='Prop2' typeName='double' kindOfQuantity='AnotherKindOfQuantity' />
            </ECEntityClass>
            <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here'
                displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3'
                presentationUnits='FT;IN;MILLIINCH'/>
            <KindOfQuantity typeName='SecondKindOfQuantity' persistenceUnit='N' relativeError='10e-3' />
            <KindOfQuantity typeName='AnotherKindOfQuantity' persistenceUnit='PA' relativeError='10e-3'
                presentationUnits='PSI'/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext)) << "ECSchema failed to deserialize EC3 schema.";
    ASSERT_TRUE(schema.IsValid());

    Utf8String convertedDuringXmlSerialization;
    ASSERT_EQ(SchemaWriteStatus::Success, ECSchema::WriteToEC2XmlString(convertedDuringXmlSerialization, schema.get()));

    ASSERT_TRUE(ECSchemaDownConverter::Convert(*schema));

    bvector<Utf8String> expectedRefSchemas;
    expectedRefSchemas.push_back("Unit_Attributes.01.00.00");
    verifyReferencedSchemas(*schema, expectedRefSchemas);
    EXPECT_STREQ("CENTIMETRE", getUnitName(schema.get(), "A", "PropA", "UnitSpecificationAttr", "UnitName").c_str());
    EXPECT_FALSE(schema->GetClassCP("A")->GetPropertyP("PropA")->GetCustomAttribute("OldPersistenceUnit").IsValid());
    EXPECT_STREQ("FOOT", getUnitName(schema.get(), "A", "PropA", "DisplayUnitSpecificationAttr", "DisplayUnitName").c_str());
    EXPECT_STREQ("NEWTON", getUnitName(schema.get(), "D", "Prop1", "UnitSpecificationAttr", "UnitName").c_str());
    EXPECT_FALSE(schema->GetClassCP("D")->GetPropertyP("Prop1")->GetCustomAttributeLocal("DisplayUnitSpecificationAttr").IsValid());
    EXPECT_STREQ("NEWTON_PER_METRE_SQUARED", getUnitName(schema.get(), "D", "Prop2", "UnitSpecificationAttr", "UnitName").c_str());
    EXPECT_STREQ("POUND_FORCE_PER_INCH_SQUARED", getUnitName(schema.get(), "D", "Prop2", "DisplayUnitSpecificationAttr", "DisplayUnitName").c_str());
    EXPECT_FALSE(schema->GetClassCP("B")->GetPropertyP("PropA")->GetCustomAttributeLocal("UnitSpecificationAttr").IsValid());
    EXPECT_FALSE(schema->GetClassCP("B")->GetPropertyP("PropA")->GetCustomAttributeLocal("DisplayUnitSpecificationAttr").IsValid());
    EXPECT_FALSE(schema->GetClassCP("C")->GetPropertyP("PropA")->GetCustomAttributeLocal("UnitSpecificationAttr").IsValid());
    EXPECT_FALSE(schema->GetClassCP("C")->GetPropertyP("PropA")->GetCustomAttributeLocal("DisplayUnitSpecificationAttr").IsValid());
    EXPECT_FALSE(schema->GetClassCP("D")->GetPropertyP("Prop0")->GetCustomAttributeLocal("UnitSpecificationAttr").IsValid());
    EXPECT_FALSE(schema->GetClassCP("D")->GetPropertyP("Prop0")->GetCustomAttributeLocal("DisplayUnitSpecificationAttr").IsValid());

    Utf8String convertedThenSerialized;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(convertedThenSerialized, ECVersion::V2_0));

    ASSERT_STREQ(convertedDuringXmlSerialization.c_str(), convertedThenSerialized.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsCustomAttributesConversionTests, EC32SchemasWithKoQsProperlyRemoveReferenceToStandardUnitsSchema)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
            <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.00" alias="V2ToV3" />
            <ECSchemaReference name="Units" version="01.00.00" alias="u" />
            <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
            <ECEntityClass typeName='A' modifier='abstract'>
                <ECProperty propertyName='PropA' typeName='double' kindOfQuantity='MyKindOfQuantity'>
                    <ECCustomAttributes>
                        <OldPersistenceUnit xmlns="ECv3ConversionAttributes.01.00.00">
                            <Name>FOOT</Name>
                        </OldPersistenceUnit>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECEntityClass typeName='B' modifer='none'>
                <BaseClass>A</BaseClass>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECEntityClass>
            <ECEntityClass typeName='C' modifer='sealed'>
                <BaseClass>B</BaseClass>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECEntityClass>
            <ECEntityClass typeName='D' modifer='sealed'>
                <ECProperty propertyName='Prop0' typeName='double' />
                <ECProperty propertyName='Prop1' typeName='double' kindOfQuantity='SecondKindOfQuantity' />
                <ECProperty propertyName='Prop2' typeName='double' kindOfQuantity='AnotherKindOfQuantity' />
            </ECEntityClass>
            <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here'
                displayLabel='best quantity of all times' persistenceUnit='u:CM' relativeError='10e-3'
                presentationFormats='f:Feet4U;f:InchesU'/>
            <KindOfQuantity typeName='SecondKindOfQuantity' persistenceUnit='u:N' relativeError='10e-3' />
            <KindOfQuantity typeName='AnotherKindOfQuantity' persistenceUnit='u:PA' relativeError='10e-3'
                presentationFormats='f:AmerFI'/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext)) << "ECSchema failed to deserialize EC3 schema.";
    ASSERT_TRUE(schema.IsValid());

    Utf8String convertedDuringXmlSerialization;
    ASSERT_EQ(SchemaWriteStatus::Success, ECSchema::WriteToEC2XmlString(convertedDuringXmlSerialization, schema.get()));

    ASSERT_TRUE(ECSchemaDownConverter::Convert(*schema));
    ASSERT_FALSE(schema->IsSchemaReferenced(*schema, *ECTestFixture::GetUnitsSchema()));
    ASSERT_FALSE(schema->IsSchemaReferenced(*schema, *ECTestFixture::GetFormatsSchema()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Joseph.Urbano                       08/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsCustomAttributesConversionTests, EC2SchemasWithoutKoQsDoNotAttemptToRemoveReferenceToStandardUnitsSchema)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
            <ECSchemaReference name='ECv3ConversionAttributes' version='01.00.00' prefix='V2ToV3' />
            <ECClass typeName='A'>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECClass>
            <ECClass typeName='B'>
                <BaseClass>A</BaseClass>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECClass>
            <ECClass typeName='C'>
                <BaseClass>B</BaseClass>
                <ECProperty propertyName='PropA' typeName='double' />
            </ECClass>
            <ECClass typeName='D'>
                <ECProperty propertyName='Prop0' typeName='double' />
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext)) << "ECSchema failed to deserialize EC3 schema.";
    ASSERT_TRUE(schema.IsValid());

    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));
    ASSERT_FALSE(schema->IsSchemaReferenced(*schema, *ECTestFixture::GetUnitsSchema()));
    ASSERT_FALSE(schema->IsSchemaReferenced(*schema, *ECTestFixture::GetFormatsSchema()));

    Utf8String convertedDuringXmlSerialization;
    ASSERT_EQ(SchemaWriteStatus::Success, ECSchema::WriteToEC2XmlString(convertedDuringXmlSerialization, schema.get()));

    ASSERT_TRUE(ECSchemaDownConverter::Convert(*schema));
    ASSERT_FALSE(schema->IsSchemaReferenced(*schema, *ECTestFixture::GetUnitsSchema()));
    ASSERT_FALSE(schema->IsSchemaReferenced(*schema, *ECTestFixture::GetFormatsSchema()));
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
    ASSERT_TRUE(ECSchemaConverter::Convert(*schema.get(), context.get())) << "Failed to convert schema";

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

    Units::Quantity lengthQ;
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->GetQuantity(lengthQ, "Length"));
    EXPECT_STREQ("M", lengthQ.GetUnitName());
    ECUnitCP unit = ECTestFixture::GetUnitsSchema()->GetUnitCP("FT");
    ASSERT_NE(nullptr, unit);
    EXPECT_EQ(50, lengthQ.ConvertTo(unit).GetMagnitude());
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
