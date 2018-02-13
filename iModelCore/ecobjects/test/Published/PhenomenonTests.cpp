/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/PhenomenonTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PhenomenonTests : ECTestFixture {};
struct PhenomenonDeserializationTest : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonTests, BasicPhenomenonCreation)
    {
    ECSchemaPtr schema;
    PhenomenonP Phenomenon;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    EC_EXPECT_SUCCESS(schema->CreatePhenomenon(Phenomenon, "TestPhenomenon", "LENGTH*LENGTH", "DisplayLabel", "TestDescription"));

    EXPECT_STREQ("TestDescription", Phenomenon->GetInvariantDescription().c_str());
    EXPECT_STREQ("DisplayLabel", Phenomenon->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", Phenomenon->GetDefinition().c_str());

    auto testPhenomenon = schema->GetPhenomenonCP("TestPhenomenon");
    EXPECT_EQ(Phenomenon, testPhenomenon);

    auto registryPhenomenon = Units::UnitRegistry::Instance().LookupPhenomenon("TestPhenomenon");
    EXPECT_EQ(Phenomenon, registryPhenomenon);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonTests, PhenomenonContainerTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    {
    PhenomenonP phenom;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePhenomenon(phenom, "Phenomenon1", "LENGTH", "Phenomenon 1", "The first Phenomenon"));
    EXPECT_TRUE(nullptr != phenom);
    }
    {
    PhenomenonP phenom;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePhenomenon(phenom, "Phenomenon2", "LENGTH*LENGTH", "Phenomenon 2", "The second Phenomenon"));
    EXPECT_TRUE(nullptr != phenom);
    }
    {
    PhenomenonP phenom;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePhenomenon(phenom, "Phenomenon3", "LENGTH*LENGTH*LENGTH", "Phenomenon 3", "The third Phenomenon"));
    EXPECT_TRUE(nullptr != phenom);
    }
    {
    PhenomenonP phenom;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePhenomenon(phenom, "Phenomenon4", "LENGTH*LENGTH*LENGTH*LENGTH", "Phenomenon 4", "The fourth Phenomenon"));
    EXPECT_TRUE(nullptr != phenom);
    }

    EXPECT_EQ(4, schema->GetPhenomenonCount());
    int curCount = 0;
    for (PhenomenonCP phenom : schema->GetPhenomena())
        {
        EXPECT_TRUE(nullptr != phenom);
        switch (curCount)
            {
            case 0:
                EXPECT_STREQ("Phenomenon1", phenom->GetName().c_str());
                EXPECT_STREQ("The first Phenomenon", phenom->GetInvariantDescription().c_str());
                EXPECT_STREQ("Phenomenon 1", phenom->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("LENGTH", phenom->GetDefinition().c_str());
                break;
            case 1:
                EXPECT_STREQ("Phenomenon2", phenom->GetName().c_str());
                EXPECT_STREQ("The second Phenomenon", phenom->GetInvariantDescription().c_str());
                EXPECT_STREQ("Phenomenon 2", phenom->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("LENGTH*LENGTH", phenom->GetDefinition().c_str());
                break;
            case 2:
                EXPECT_STREQ("Phenomenon3", phenom->GetName().c_str());
                EXPECT_STREQ("The third Phenomenon", phenom->GetInvariantDescription().c_str());
                EXPECT_STREQ("Phenomenon 3", phenom->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("LENGTH*LENGTH*LENGTH", phenom->GetDefinition().c_str());
                break;
            case 3:
                EXPECT_STREQ("Phenomenon4", phenom->GetName().c_str());
                EXPECT_STREQ("The fourth Phenomenon", phenom->GetInvariantDescription().c_str());
                EXPECT_STREQ("Phenomenon 4", phenom->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("LENGTH*LENGTH*LENGTH*LENGTH", phenom->GetDefinition().c_str());
                break;
            }

        curCount++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PhenomenonTests, StandaloneSchemaChildPhenomenon)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    PhenomenonP phenom;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH*LENGTH", "ExamplePhenomenonLabel", "ExamplePhenomenonDescription");

    Json::Value schemaJson;
    EXPECT_EQ(SchemaWriteStatus::Success, phenom->WriteJson(schemaJson, true));

    Json::Value testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandalonePhenomenon.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonDeserializationTest, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetPhenomenonCount());
    PhenomenonCP phenom = schema->GetPhenomenonCP("TestPhenomenon");
    ASSERT_TRUE(nullptr != phenom);
    ASSERT_EQ(&phenom->GetSchema(), schema.get());

    EXPECT_STREQ("Phenomenon", phenom->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Phenomenon", phenom->GetInvariantDescription().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", phenom->GetDefinition().c_str());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetPhenomenonCount());
    PhenomenonCP serializedPhenomenon = serializedSchema->GetPhenomenonCP("TestPhenomenon");
    ASSERT_TRUE(nullptr != serializedPhenomenon);

    EXPECT_STREQ("Phenomenon", serializedPhenomenon->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Phenomenon", serializedPhenomenon->GetInvariantDescription().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedPhenomenon->GetDefinition().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonDeserializationTest, DuplicatePhenomenonNames)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonDeserializationTest, DuplicateSchemaChildNames)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="TestPhenomenon"/>
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonDeserializationTest, MissingOrInvalidName)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon without a name"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon with an empty name"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonDeserializationTest, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="aUniquePhenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    PhenomenonCP Phenomenon = schema->GetPhenomenonCP("aUniquePhenomenon");
    EXPECT_STREQ("aUniquePhenomenon", Phenomenon->GetInvariantDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonDeserializationTest, MissingOrEmptyDefinition)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="aUniquePhenomenon" displayLabel="Phenomenon" description="This is an awesome new Phenomenon"/>
    </ECSchema>)xml";
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="aUniquePhenomenon" displayLabel="Phenomenon" definition="" description="This is an awesome new Phenomenon"/>
    </ECSchema>)xml";
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml2, *context));
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
