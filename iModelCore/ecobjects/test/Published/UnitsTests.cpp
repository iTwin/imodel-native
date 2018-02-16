/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/UnitsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitsTests : ECTestFixture {};
struct UnitsDeserializationTests : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, BasicECUnitCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    EC_EXPECT_SUCCESS(schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, "ExampleUnitLabel", "ExampleUnitDescription", 10.0, 1.0)); 
    
    EXPECT_STREQ("ExampleUnitDescription", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("ExampleUnitLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("ExampleUnit", unit->GetName().c_str());
    EXPECT_EQ(phenom, unit->GetPhenomenon());
    EXPECT_EQ(system, unit->GetUnitSystem());
    EXPECT_EQ(10.0, unit->GetFactor());
    EXPECT_EQ(1.0, unit->GetOffset());

    auto testECUnit = schema->GetUnitCP("ExampleUnit");
    EXPECT_EQ(unit, testECUnit);

    auto registryECUnit = Units::UnitRegistry::Instance().LookupUnit("TestSchema:ExampleUnit");
    EXPECT_EQ(unit, registryECUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, ECUnitContainerTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success,schema->CreateUnit(unit, "ExampleUnit1", "M", *phenom, *system, "ExampleUnitLabel1", "ExampleUnitDescription1"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit2", "MM", *phenom, *system, "ExampleUnitLabel2", "ExampleUnitDescription2"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit3", "MMM", *phenom, *system, "ExampleUnitLabel3", "ExampleUnitDescription3"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit4", "MMMM", *phenom, *system, "ExampleUnitLabel4", "ExampleUnitDescription4"));
    EXPECT_TRUE(nullptr != unit);
    }

    EXPECT_EQ(4, schema->GetUnitCount());
    int curCount = 0;
    for (ECUnitCP unit : schema->GetUnits())
        {
        EXPECT_TRUE(nullptr != unit);
        switch (curCount)
            {
            case 0:
                EXPECT_STREQ("ExampleUnitDescription1", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel1", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("M", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit1", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_EQ(1.0, unit->GetFactor());
                EXPECT_EQ(0.0, unit->GetOffset());
                break;
            case 1:
                EXPECT_STREQ("ExampleUnitDescription2", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel2", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit2", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_EQ(1.0, unit->GetFactor());
                EXPECT_EQ(0.0, unit->GetOffset());
                break;
            case 2:
                EXPECT_STREQ("ExampleUnitDescription3", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel3", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MMM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit3", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_EQ(1.0, unit->GetFactor());
                EXPECT_EQ(0.0, unit->GetOffset());
                break;
            case 3:
                EXPECT_STREQ("ExampleUnitDescription4", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel4", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MMMM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit4", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_EQ(1.0, unit->GetFactor());
                EXPECT_EQ(0.0, unit->GetOffset());
                break;
            }
        curCount++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, StandaloneSchemaChildECUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, "ExampleUnitLabel", "ExampleUnitDescription", 10.0, 1.0);

    Json::Value schemaJson;
    EXPECT_EQ(SchemaWriteStatus::Success, unit->WriteJson(schemaJson, true));

    Json::Value testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneUnit.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedECUnit);

    EXPECT_STREQ("Unit", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, RoundTripWithReferencedSchemaForPhenomenonAndUnitSystem)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="refSchema" version="01.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());
        
    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)unit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("Unit", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedUnit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)serializedUnit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithoutAliasesForPhenomenonAndUnitSystemFromReferencedSchema)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="refSchema" version="01.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithoutPhenomenonAndUnitSystemBeingDefinedInThisOrAnyReferencedSchema)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, DuplicateUnitNames)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, DuplicateSchemaChildNames)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="TestUnit"/>
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidName)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    EXPECT_STREQ("TestUnit", unit->GetInvariantDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrEmptyDefinition)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" description="This is an awesome new Unit"/>
    </ECSchema>)xml";
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="" description="This is an awesome new Unit"/>
    </ECSchema>)xml";
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml2, *context));
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
