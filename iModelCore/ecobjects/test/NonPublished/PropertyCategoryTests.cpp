/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PropertyCategoryTest : ECTestFixture {};
struct PropertyCategoryDeserializationTest : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCategoryTest, BasicPropertyCategoryCreation)
    {
    ECSchemaPtr schema;
    PropertyCategoryP propCategory;
    
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(propCategory, "PropertyCategory"));

    EXPECT_EQ(ECObjectsStatus::Success, propCategory->SetDescription("TestDescription"));
    EXPECT_STREQ("TestDescription", propCategory->GetDescription().c_str());
    
    EXPECT_EQ(ECObjectsStatus::Success, propCategory->SetDisplayLabel("DisplayLabel"));
    EXPECT_STREQ("DisplayLabel", propCategory->GetDisplayLabel().c_str());

    EXPECT_EQ(ECObjectsStatus::Success, propCategory->SetPriority(2));
    EXPECT_EQ(2, propCategory->GetPriority());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCategoryTest, PropertyCategoryContainer)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    {
    PropertyCategoryP propCategory;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(propCategory, "PropertyCategory1"));
    EXPECT_TRUE(nullptr != propCategory);
    propCategory->SetDescription("The first PropertyCategory with priority of 2");
    propCategory->SetDisplayLabel("PropertyCategory_Priority2");
    propCategory->SetPriority(2);
    }
    {
    PropertyCategoryP propCategory;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(propCategory, "PropertyCategory2"));
    EXPECT_TRUE(nullptr != propCategory);
    propCategory->SetDescription("The second PropertyCategory with priority of 0");
    propCategory->SetDisplayLabel("PropertyCategory_Priority0");
    propCategory->SetPriority(0);
    }
    {
    PropertyCategoryP propCategory;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(propCategory, "PropertyCategory3"));
    EXPECT_TRUE(nullptr != propCategory);
    propCategory->SetDescription("The third PropertyCategory with priority of 1");
    propCategory->SetDisplayLabel("PropertyCategory_Priority1");
    propCategory->SetPriority(1);
    }
    {
    PropertyCategoryP propCategory;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(propCategory, "PropertyCategory4"));
    EXPECT_TRUE(nullptr != propCategory);
    propCategory->SetDescription("The fourth PropertyCategory with priority of 3");
    propCategory->SetDisplayLabel("PropertyCategory_Priority3");
    propCategory->SetPriority(3);
    }

    EXPECT_EQ(4, schema->GetPropertyCategoryCount());
    int curCount = 0;
    for (PropertyCategoryCP propCategory : schema->GetPropertyCategories())
        {
        EXPECT_TRUE(nullptr != propCategory);
        switch (curCount)
            {
            case 0:
                EXPECT_STREQ("PropertyCategory1", propCategory->GetName().c_str());
                EXPECT_STREQ("The first PropertyCategory with priority of 2", propCategory->GetDescription().c_str());
                EXPECT_STREQ("PropertyCategory_Priority2", propCategory->GetDisplayLabel().c_str());
                EXPECT_EQ(2, propCategory->GetPriority());
                break;
            case 1:
                EXPECT_STREQ("PropertyCategory2", propCategory->GetName().c_str());
                EXPECT_STREQ("The second PropertyCategory with priority of 0", propCategory->GetDescription().c_str());
                EXPECT_STREQ("PropertyCategory_Priority0", propCategory->GetDisplayLabel().c_str());
                EXPECT_EQ(0, propCategory->GetPriority());
                break;
            case 2:
                EXPECT_STREQ("PropertyCategory3", propCategory->GetName().c_str());
                EXPECT_STREQ("The third PropertyCategory with priority of 1", propCategory->GetDescription().c_str());
                EXPECT_STREQ("PropertyCategory_Priority1", propCategory->GetDisplayLabel().c_str());
                EXPECT_EQ(1, propCategory->GetPriority());
                break;
            case 3:
                EXPECT_STREQ("PropertyCategory4", propCategory->GetName().c_str());
                EXPECT_STREQ("The fourth PropertyCategory with priority of 3", propCategory->GetDescription().c_str());
                EXPECT_STREQ("PropertyCategory_Priority3", propCategory->GetDisplayLabel().c_str());
                EXPECT_EQ(3, propCategory->GetPriority());
                break;
            }

        curCount++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyCategoryTest, StandaloneSchemaItemPropertyCategory)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    PropertyCategoryP prop;
    schema->CreatePropertyCategory(prop, "ExamplePropertyCategory");
    prop->SetPriority(5);

    Json::Value schemaJson;
    EXPECT_TRUE(prop->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandalonePropertyCategory.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  05/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCategoryTest, LookupPropertyCategoryTest)
    {
    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ref" version="01.00.00" alias="r" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="propCategory" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="2"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ref" version="1.0.0" alias="r"/>
            <PropertyCategory typeName="propCategory" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="2"/>
        </ECSchema>)xml";

    ECSchemaPtr refSchema;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    auto shouldBeNull = schema->LookupPropertyCategory("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupPropertyCategory("banana");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupPropertyCategory("banana:propCategory");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupPropertyCategory("testSchema:propCategory");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupPropertyCategory("ref:propCategory");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupPropertyCategory("r:propCategory", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = schema->LookupPropertyCategory("propCategory");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("ts:propCategory");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("testSchema:propCategory", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("r:propCategory");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("ref:propCategory", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("TS:propCategory");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("TESTSCHEMA:propCategory", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("R:propCategory");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupPropertyCategory("REF:propCategory", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("propCategory", shouldNotBeNull->GetName().c_str());
    ASSERT_EQ(1, schema->GetPropertyCategoryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCategoryDeserializationTest, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <PropertyCategory typeName="propCategory" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="2"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    EXPECT_EQ(1, schema->GetPropertyCategoryCount());
    PropertyCategoryCP propCategory = schema->GetPropertyCategoryCP("propCategory");
    EXPECT_TRUE(nullptr != propCategory);

    EXPECT_STREQ("PropertyCategory", propCategory->GetDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Property Category", propCategory->GetDescription().c_str());
    EXPECT_EQ(2, propCategory->GetPriority());

    Utf8String serializedSchemaXml;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    EXPECT_EQ(1, serializedSchema->GetPropertyCategoryCount());
    PropertyCategoryCP serializedPropCategory = serializedSchema->GetPropertyCategoryCP("propCategory");
    EXPECT_TRUE(nullptr != serializedPropCategory);

    EXPECT_STREQ("PropertyCategory", serializedPropCategory->GetDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Property Category", serializedPropCategory->GetDescription().c_str());
    EXPECT_EQ(2, serializedPropCategory->GetPriority());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCategoryDeserializationTest, MissingOrInvalidName)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <PropertyCategory displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <PropertyCategory typeName="" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <PropertyCategory typeName="" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCategoryDeserializationTest, MissingDisplayLabel)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <PropertyCategory typeName="aUniquePropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    PropertyCategoryCP propCat = schema->GetPropertyCategoryCP("aUniquePropertyCategory");
    EXPECT_STREQ("aUniquePropertyCategory", propCat->GetDisplayLabel().c_str());
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE