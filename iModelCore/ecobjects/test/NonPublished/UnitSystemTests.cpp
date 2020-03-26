/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitSystemTest : ECTestFixture {};
struct UnitSystemDeserializationTest : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemTest, BasicUnitSystemCreation)
    {
    ECSchemaPtr schema;
    UnitSystemP unitSystem;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    EC_EXPECT_SUCCESS(schema->CreateUnitSystem(unitSystem, "TestUnitSystem", "DisplayLabel", "TestDescription"));

    EXPECT_STREQ("TestDescription", unitSystem->GetInvariantDescription().c_str());
    EXPECT_STREQ("DisplayLabel", unitSystem->GetInvariantDisplayLabel().c_str());

    auto testUnitSystem = schema->GetUnitSystemCP("TestUnitSystem");
    EXPECT_EQ(unitSystem, testUnitSystem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemTest, LookupSystemTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <UnitSystem typeName="TestSystem"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    auto unitContext = &schema->GetUnitsContext();
    auto shouldBeNull = schema->LookupUnitSystem("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnitSystem("banana:M");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnitSystem("Units:M");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnitSystem("u:M", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = schema->LookupUnitSystem("TestSystem");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestSystem", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("ts:TestSystem");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestSystem", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("TestSchema:TestSystem", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestSystem", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("u:SI");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("SI", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("Units:SI", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("SI", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("TS:TestSystem");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestSystem", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("TESTSCHEMA:TestSystem", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestSystem", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("U:SI");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("SI", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnitSystem("UNITS:SI", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("SI", shouldNotBeNull->GetName().c_str());
    bvector<Units::UnitSystemCP> systems;
    unitContext->AllSystems(systems);
    ASSERT_EQ(schema->GetUnitSystemCount(), systems.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemTest, UnitSystemContainer)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    {
    UnitSystemP unitSystem;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnitSystem(unitSystem, "UnitSystem1", "Unit System 1", "The first UnitSystem"));
    EXPECT_TRUE(nullptr != unitSystem);
    }
    {
    UnitSystemP unitSystem;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnitSystem(unitSystem, "UnitSystem2", "Unit System 2", "The second UnitSystem"));
    EXPECT_TRUE(nullptr != unitSystem);
    }
    {
    UnitSystemP unitSystem;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnitSystem(unitSystem, "UnitSystem3", nullptr, "The third UnitSystem"));
    EXPECT_TRUE(nullptr != unitSystem);
    }
    {
    UnitSystemP unitSystem;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnitSystem(unitSystem, "UnitSystem4", "Unit System 4", "The fourth UnitSystem"));
    EXPECT_TRUE(nullptr != unitSystem);
    }

    EXPECT_EQ(4, schema->GetUnitSystemCount());
    int curCount = 0;
    for (UnitSystemCP unitSystem : schema->GetUnitSystems())
        {
        EXPECT_TRUE(nullptr != unitSystem);
        switch (curCount)
            {
            case 0:
                EXPECT_STREQ("UnitSystem1", unitSystem->GetName().c_str());
                EXPECT_STREQ("The first UnitSystem", unitSystem->GetInvariantDescription().c_str());
                EXPECT_STREQ("Unit System 1", unitSystem->GetInvariantDisplayLabel().c_str());
                break;
            case 1:
                EXPECT_STREQ("UnitSystem2", unitSystem->GetName().c_str());
                EXPECT_STREQ("The second UnitSystem", unitSystem->GetInvariantDescription().c_str());
                EXPECT_STREQ("Unit System 2", unitSystem->GetInvariantDisplayLabel().c_str());
                break;
            case 2:
                EXPECT_STREQ("UnitSystem3", unitSystem->GetName().c_str());
                EXPECT_STREQ("The third UnitSystem", unitSystem->GetInvariantDescription().c_str());
                EXPECT_STREQ("UnitSystem3", unitSystem->GetInvariantDisplayLabel().c_str());
                break;
            case 3:
                EXPECT_STREQ("UnitSystem4", unitSystem->GetName().c_str());
                EXPECT_STREQ("The fourth UnitSystem", unitSystem->GetInvariantDescription().c_str());
                EXPECT_STREQ("Unit System 4", unitSystem->GetInvariantDisplayLabel().c_str());
                break;
            }

        curCount++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitSystemTest, StandaloneSchemaChildUnitSystem)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    UnitSystemP system;
    schema->CreateUnitSystem(system, "ExampleUnitSystem", "ExampleUnitSystemLabel", "ExampleUnitSystemDescription");

    Json::Value schemaJson;
    EXPECT_TRUE(system->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneUnitSystem.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemDeserializationTest, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitSystemCount());
    UnitSystemCP unitSystem = schema->GetUnitSystemCP("TestUnitSystem");
    ASSERT_TRUE(nullptr != unitSystem);
    ASSERT_EQ(&unitSystem->GetSchema(), schema.get());

    EXPECT_STREQ("Unit System", unitSystem->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit System", unitSystem->GetInvariantDescription().c_str());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitSystemCount());
    UnitSystemCP serializedUnitSystem = serializedSchema->GetUnitSystemCP("TestUnitSystem");
    ASSERT_TRUE(nullptr != serializedUnitSystem);

    EXPECT_STREQ("Unit System", serializedUnitSystem->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit System", serializedUnitSystem->GetInvariantDescription().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemDeserializationTest, EmptyDisplayLabelRoundTripTest)
{
	Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TestUnitSystem" displayLabel="" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";

	Utf8String serializedSchemaXml;
	ECSchemaPtr schema;
	ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
	ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

	UnitSystemCP unitSystem = schema->GetUnitSystemCP("TestUnitSystem");
	ASSERT_TRUE(nullptr != unitSystem);
	EXPECT_STREQ("TestUnitSystem", unitSystem->GetInvariantDisplayLabel().c_str());

	EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
	ASSERT_FALSE(serializedSchemaXml.Contains("displayLabel=\"TestUnitSystem\""));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemDeserializationTest, MissingOrInvalidName)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <UnitSystem displayLabel="Unit System" description="This is an awesome new Unit System without a name"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <UnitSystem typeName="" displayLabel="Unit System" description="This is an awesome new Unit System with an empty name"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <UnitSystem typeName="....." displayLabel="Unit System" description="This is an awesome new Unit System with an empty name"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemDeserializationTest, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <UnitSystem typeName="aUniqueUnitSystem" description="This is an awesome new Unit System"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    UnitSystemCP unitSystem = schema->GetUnitSystemCP("aUniqueUnitSystem");
    EXPECT_STREQ("aUniqueUnitSystem", unitSystem->GetInvariantDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemDeserializationTest, MissingDescription)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <UnitSystem typeName="aUniqueUnitSystem"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    UnitSystemCP unitSystem = schema->GetUnitSystemCP("aUniqueUnitSystem");
    EXPECT_TRUE(unitSystem->GetInvariantDescription().empty());
    }

END_BENTLEY_ECN_TEST_NAMESPACE