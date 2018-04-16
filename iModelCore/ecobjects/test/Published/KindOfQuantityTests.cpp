/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/KindOfQuantityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct KindOfQuantityTest : ECTestFixture
    {
    ECSchemaPtr m_schema;

    void CreateTestSchema(bool refUnitSchema = false)
        {
        EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_schema, "TestSchema", "TS", 1, 0, 0));
        ASSERT_TRUE(m_schema.IsValid());

        if (refUnitSchema)
            EC_EXPECT_SUCCESS(m_schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema(true)));
        }
    };
struct KindOfQuantityRoundTripTest : ECTestFixture {};
struct KindOfQuantityUpgradeTest : ECTestFixture {};
struct KindOfQuantityCompatibilityTest : ECTestFixture {};
struct KindOfQuantityDeserializationTest : ECTestFixture {};
struct KindOfQuantitySerializationTest : ECTestFixture 
    {
    static Utf8CP s_testSchemaXml;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Basanta.Kharel   12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(KindOfQuantityTest, KindOfQuantityTestTest)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    PrimitiveECPropertyP prop;
    KindOfQuantityP koq;

    ECSchema::CreateSchema(schema, "KindOfQuantitySchema", "koq", 5, 0, 6);
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));

    schema->CreateEntityClass(entityClass, "Class");
    entityClass->CreatePrimitiveProperty(prop, "Property");

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("FT"));
    prop->SetKindOfQuantity(koq);

    Utf8String schemaXML;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXML, ECVersion::Latest));

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECClassCP entityClassDup = refSchema->GetClassCP("Class");
    ASSERT_NE(entityClassDup, nullptr);
    PrimitiveECPropertyCP property = entityClassDup->GetPropertyP("Property")->GetAsPrimitiveProperty();
    ASSERT_NE(property, nullptr);

    auto resultKindOfQuantity = property->GetKindOfQuantity();
    ASSERT_NE(resultKindOfQuantity, nullptr);
    EXPECT_STREQ("MyKindOfQuantity", resultKindOfQuantity->GetName().c_str());
    SchemaKey key = SchemaKey("Units", 1, 0, 0);
    auto unitSchema = schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_EQ(unitSchema->GetUnitCP("FT"), resultKindOfQuantity->GetPersistenceUnit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, AddRemovePresentationFormats)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema(true)));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema(true)));
    {
    KindOfQuantityP kindOfQuantity;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    EC_EXPECT_SUCCESS(kindOfQuantity->SetDescription("Kind of a Description here"));
    EC_EXPECT_SUCCESS(kindOfQuantity->SetDisplayLabel("best quantity of all times"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    EXPECT_NE(ECObjectsStatus::Success, kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetConstantCP("PI"))); // constant
    kindOfQuantity->SetRelativeError(10e-3);
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("InchesU")));
    EC_EXPECT_SUCCESS(kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("Feet4U")));
    EXPECT_NE(ECObjectsStatus::Success, kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("AngleDM")));
    }

    {
    KindOfQuantityP koq = schema->GetKindOfQuantityP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STREQ("f:InchesU", koq->GetDefaultPresentationFormat()->GetName().c_str());
    auto const& presUnitList = koq->GetPresentationFormatList();

    EXPECT_EQ(2, presUnitList.size());
    EXPECT_STREQ("f:InchesU", presUnitList.at(0).GetName().c_str());
    EXPECT_STREQ("f:Feet4U", presUnitList.at(1).GetName().c_str());
    }

    {
    KindOfQuantityP koq = schema->GetKindOfQuantityP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    auto const& presUnitList = koq->GetPresentationFormatList();

    koq->RemovePresentationFormat(presUnitList.at(1));
    EXPECT_EQ(1, presUnitList.size());
    EXPECT_STRCASEEQ("f:Feet4U", presUnitList.at(0).GetName().c_str());

    koq->RemovePresentationFormat(presUnitList.at(0));
    EXPECT_EQ(0, presUnitList.size());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, RemoveAllPresentationFormats)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));
    
    KindOfQuantityP kindOfQuantity;

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDescription("Kind of a Description here"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDisplayLabel("best quantity of all times"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    kindOfQuantity->SetRelativeError(10e-3);
    
    EXPECT_EQ(0, kindOfQuantity->GetPresentationFormatList().size());

    EC_EXPECT_SUCCESS(kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("Feet4U")));
    EXPECT_EQ(1, kindOfQuantity->GetPresentationFormatList().size());
    kindOfQuantity->RemoveAllPresentationFormats();
    EXPECT_EQ(0, kindOfQuantity->GetPresentationFormatList().size());

    EC_EXPECT_SUCCESS(kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("Feet4U")));
    EC_EXPECT_SUCCESS(kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("InchesU")));
    EXPECT_EQ(2, kindOfQuantity->GetPresentationFormatList().size());

    kindOfQuantity->RemoveAllPresentationFormats();
    EXPECT_EQ(0, kindOfQuantity->GetPresentationFormatList().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, TestIncompatiblePersistenceAndPresentationFormats)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));

    ECUnitCP meterUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP("M");
    ECUnitCP centimeterUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP("CM");
    ECFormatCP angleDm = ECTestFixture::GetFormatsSchema()->GetFormatCP("AngleDMS");
    ECFormatCP feet4u = ECTestFixture::GetFormatsSchema()->GetFormatCP("Feet4U");
    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    EXPECT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*meterUnit));
    EXPECT_NE(ECObjectsStatus::Success, koq->AddPresentationFormat(*angleDm)) << "The input unit ARC_DEG is from a different phenomeonon than the Persistence Unit.";

    KindOfQuantityP koq2;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq2, "MyKindOfQuantity2"));
    EC_EXPECT_SUCCESS(koq2->AddPresentationFormat(*angleDm)) << "The input unit ARC_DEG is from a different phenomeonon than the Persistence Unit.";
    EXPECT_NE(ECObjectsStatus::Success, koq2->SetPersistenceUnit(*meterUnit)) << "Meter is from a different phenomenon than the presentation unit";

    KindOfQuantityP koq3;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq3, "MyKindOfQuantity3"));
    EC_EXPECT_SUCCESS(koq->AddPresentationFormat(*feet4u));
    EXPECT_EQ(ECObjectsStatus::Success, koq3->SetPersistenceUnit(*centimeterUnit));
    EXPECT_NE(ECObjectsStatus::Success, koq3->AddPresentationFormat(*angleDm)) << "The input unit ARC_DEG is from a different phenomeonon than the Persistence Unit and presentation unit.";

    KindOfQuantityP koq4;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq4, "MyKindOfQuantity4"));
    EC_EXPECT_SUCCESS(koq4->AddPresentationFormat(*angleDm));
    EXPECT_NE(ECObjectsStatus::Success, koq4->AddPresentationFormat(*feet4u)) << "The input Unit FT is from the LENGTH Phenomenon which is different than the existing Presentation Unit.";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, PresentationUnitDescriptor)
    {
    CreateTestSchema(true);

    // already added as a reference schema
    auto unitSchema = ECTestFixture::GetUnitsSchema();
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    auto ftUnit = unitSchema->GetUnitCP("FT");

    ECUnitP smoot;
    m_schema->CreateUnit(smoot, "Smoot", "M", *unitSchema->GetPhenomenonCP("Length"), *unitSchema->GetUnitSystemCP("SI"), 1.7018);
    ECFormatP format;
    Formatting::NumericFormatSpec spec = Formatting::NumericFormatSpec();
    m_schema->CreateFormat(format, "SmootFormat", nullptr, nullptr, &spec);

    {
    KindOfQuantityP koq;
    m_schema->CreateKindOfQuantity(koq, "Inch_In_RefSchema_Without_Format");
    koq->SetPersistenceUnit(*ftUnit);

    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"));

    EXPECT_STREQ("f:InchesU", koq->GetPresentationUnitDescriptor().c_str()) << "The descriptor should use the qualified presentation format name";
    }
    {
    KindOfQuantityP koq;
    m_schema->CreateKindOfQuantity(koq, "Inch_In_RefSchema_With_Format");
    koq->SetPersistenceUnit(*ftUnit);
    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"), 8, unitSchema->GetUnitCP("IN"), "banana");

    EXPECT_STREQ("f:InchesU(8)[u:IN|banana]", koq->GetPresentationUnitDescriptor().c_str()) << "The descriptor should use the qualified presentation format name and the format name";
    }
    {
    KindOfQuantityP koq;
    m_schema->CreateKindOfQuantity(koq, "Smoot_In_Schema_Without_Format");
    koq->SetPersistenceUnit(*ftUnit);
    koq->AddPresentationFormatSingleUnitOverride(*format);

    EXPECT_STREQ("SmootFormat", koq->GetPresentationUnitDescriptor().c_str()) << "The descriptor should use the qualified presentation format name";
    }
    {
    KindOfQuantityP koq;
    m_schema->CreateKindOfQuantity(koq, "Smoot_In_Schema_With_Format");
    koq->SetPersistenceUnit(*ftUnit);
    koq->AddPresentationFormatSingleUnitOverride(*format, nullptr, smoot, "banana");;

    EXPECT_STREQ("SmootFormat[Smoot|banana]", koq->GetPresentationUnitDescriptor().c_str()) << "The descriptor should use the qualified presentation format name and the format name";
    }
    {
    KindOfQuantityP koq;
    m_schema->CreateKindOfQuantity(koq, "No_Unit_Or_Format");

    EXPECT_STREQ("", koq->GetPresentationUnitDescriptor().c_str()) << "The descriptor should be empty when no presentation Units are defined.";
    }
    {
    KindOfQuantityP koq;
    m_schema->CreateKindOfQuantity(koq, "Multiple_Units_and_Format");
    koq->SetPersistenceUnit(*ftUnit);
    koq->AddPresentationFormatSingleUnitOverride(*format, nullptr, smoot, "banana");
    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"), 8, unitSchema->GetUnitCP("IN"), "banana");

    EXPECT_STREQ("SmootFormat[Smoot|banana];f:InchesU(8)[u:IN|banana]", koq->GetPresentationUnitDescriptor().c_str()) << "The descriptor should use the qualified presentation unit name and the format name";
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, UpdateFUSDescriptor)
    {
    Utf8String persUnit;
    Utf8String presString;
    {
    persUnit.clear();
    presString.clear();
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, nullptr, nullptr));
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, nullptr, ""));
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "", ""));
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "", presString.c_str()));
    EXPECT_TRUE(persUnit.empty());
    EXPECT_TRUE(presString.empty());
    }
    {
    persUnit.clear();
    presString.clear();
    EXPECT_EQ(ECObjectsStatus::InvalidUnitName, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "badUnit", "MM"));
    EXPECT_TRUE(persUnit.empty());
    EXPECT_TRUE(presString.empty());
    }
    {
    persUnit.clear();
    presString.clear();
    EXPECT_EQ(ECObjectsStatus::InvalidUnitName, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM", "badUnit"));
    EXPECT_TRUE(persUnit.empty());
    EXPECT_TRUE(presString.empty());
    }
    {
    persUnit.clear();
    presString.clear();
    EXPECT_EQ(ECObjectsStatus::InvalidFormat, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM", "MM(KindOfFormat)"));
    EXPECT_TRUE(persUnit.empty());
    EXPECT_TRUE(presString.empty());
    }
    {
    persUnit.clear();
    presString.clear();
    EXPECT_EQ(ECObjectsStatus::InvalidFormat, KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM(KindOfFormat)", "MM"));
    EXPECT_TRUE(persUnit.empty());
    EXPECT_TRUE(presString.empty());
    }
    {
    persUnit.clear();
    presString.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM", nullptr));
    EXPECT_STRCASEEQ("u:MM", persUnit.c_str());
    }
    {
    persUnit.clear();
    presString.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM(real)", nullptr));
    EXPECT_STRCASEEQ("u:MM", persUnit.c_str());
    EXPECT_STRCASEEQ("f:DefaultReal[u:MM]", presString.c_str());
    }
    {
    persUnit.clear();
    presString.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM(real)", "CM"));
    EXPECT_STRCASEEQ("u:MM", persUnit.c_str());
    EXPECT_STRCASEEQ("f:DefaultRealU[u:CM];f:DefaultReal[u:MM]", presString.c_str());
    }
    {
    persUnit.clear();
    presString.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM(real)", "CM(real4u)"));
    EXPECT_STRCASEEQ("u:MM", persUnit.c_str());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM];f:DefaultReal[u:MM]", presString.c_str());
    }
    {
    persUnit.clear();
    presString.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM", "CM(real4u)"));
    EXPECT_STRCASEEQ("u:MM", persUnit.c_str());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM];f:DefaultRealU[u:MM]", presString.c_str());
    }
    {
    persUnit.clear();
    presString.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnit, presString, "MM(real)", "DM(real);CM(real4u)"));
    EXPECT_STRCASEEQ("u:MM", persUnit.c_str());
    EXPECT_STRCASEEQ("f:DefaultReal[u:DM];f:DefaultRealU(4)[u:CM];f:DefaultReal[u:MM]", presString.c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, AddPersistenceUnitByName)
    {
    KindOfQuantityP koq;
    static const auto unitLookerUpper = [&](Utf8StringCR name)
        {
        return koq->GetSchema().GetUnitsContext().LookupUnit(name.c_str());
        };

    static const auto formatLookerUpper = [&](Utf8StringCR name)
        {
        return koq->GetSchema().LookupFormat(name.c_str());
        };

    {
    CreateTestSchema(true);
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    koq->AddPersitenceUnitByName("u:M", unitLookerUpper);
    EXPECT_STRCASEEQ("M", koq->GetPersistenceUnit()->GetName().c_str());
    }

    {
    CreateTestSchema(true);
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    koq->AddPersitenceUnitByName("u:fakeUnit", unitLookerUpper);
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    }

    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, AddPresentationFormatByString)
    {
    KindOfQuantityP koq;

    static const auto unitLookerUpper = [&](Utf8StringCR name)
        {
        return koq->GetSchema().GetUnitsContext().LookupUnit(name.c_str());
        };

    static const auto formatLookerUpper = [&](Utf8StringCR name)
        {
        return koq->GetSchema().LookupFormat(name.c_str());
        };

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetDefaultPresentationFormat());
    koq->AddPresentationFormatsByString("f:InchesU", formatLookerUpper, unitLookerUpper);
    EXPECT_STRCASEEQ("f:InchesU", koq->GetDefaultPresentationFormat()->GetName().c_str());
    }

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetDefaultPresentationFormat());
    koq->AddPresentationFormatsByString("f:InchesU(8)", formatLookerUpper, unitLookerUpper);
    EXPECT_STRCASEEQ("f:InchesU(8)", koq->GetDefaultPresentationFormat()->GetName().c_str());
    }

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetDefaultPresentationFormat());
    koq->SetPersistenceUnit(*m_schema->GetUnitsContext().LookupUnit("u:M"));
    koq->AddPresentationFormatsByString("f:InchesU(8)[u:IN|inch(es)]", formatLookerUpper, unitLookerUpper);
    EXPECT_STRCASEEQ("f:InchesU(8)[u:IN|inch(es)]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(KindOfQuantityTest, SerializeStandaloneItemKindOfQuantity)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "ExampleKoQ");
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    koq->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("Feet4U"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));
    koq->SetRelativeError(3);

    Json::Value schemaJson;
    EXPECT_EQ(SchemaWriteStatus::Success, koq->WriteJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneKindOfQuantity.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//=======================================================================================
//! KindOfQuantityDeserializationTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestEmptyOrMissingName)
    {
    // Missing name
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="CM" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a missing name");
    // Empty name
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity typeName="" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="CM" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestEmptyOrMissingRelativeError)
    {
    // Missing relative error
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity typeName="TestKOQ" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="CM"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a missing relative error");
    // Empty relative error
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity typeName="TestKOQ" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="CM" relativeError=""/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty relative error");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestEmptyOrMissingPersistenceUnit)
    {
    // Missing persistence unit
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a missing persistence unit");
    // Empty persistence unit
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty persistence unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestConstantAsPersistanceUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="PI" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a constant as a persistence unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestUnitInSchemaAsPersistenceUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="TestUnit" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    auto unit = schema->GetUnitCP("TestUnit");
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    auto koqPerUnit = koq->GetPersistenceUnit();
    ASSERT_EQ(unit, koqPerUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestUnitInReferencedSchemaAsPersistenceUnit)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="Smoot" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP koqSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="koqSchema" alias="ks" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="testSchema" version="01.00" alias="ts"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="ts:TestUnit" relativeError="10e-3"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaPtr koqSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(koqSchema, koqSchemaXml, *context));
    auto koq = koqSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    auto unit = schema->GetUnitCP("TestUnit");
    auto koqPerUnit = koq->GetPersistenceUnit();
    ASSERT_EQ(unit, koqPerUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestIncompatiblePersistenceAndPresentationFormats)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX" presentationUnits="M" />
        </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="LUX"/>
        </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="CM;LUX"/>
    </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX" presentationUnits="CM;MM"/>
    </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenKindOfQuantityIsAppliedToStructAndStructArrayPropertes)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <ECStructProperty propertyName='StructWithKOQ' typeName='S' kindOfQuantity='MyKindOfQuantity' />"
        "       <ECStructArrayProperty propertyName='StructArrayWithKOQ' typeName='S' kindOfQuantity='MyKindOfQuantity' />"
        "   </ECEntityClass>"
        "   <ECStructClass typeName='S'>"
        "       <ECProperty propertyName='S_P' typeName='string' />"
        "   </ECStructClass>"
        "   <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here' "
        "       displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3' "
        "       presentationUnits='FT;IN;MILLIINCH'/>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_2));

    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_NE(nullptr, aClass) << "Could not find 'A' class";
    ECPropertyP structProp = aClass->GetPropertyP("StructWithKOQ");
    ASSERT_NE(nullptr, structProp) << "Can't find 'StructWithKOQ' property";
    KindOfQuantityCP koq = structProp->GetKindOfQuantity();
    ASSERT_NE(nullptr, koq) << "'StructWithKOQ' property does not have a KOQ as expected";
    ECPropertyP structArrayProp = aClass->GetPropertyP("StructArrayWithKOQ");
    ASSERT_NE(nullptr, structArrayProp) << "Can't find 'StructArrayWithKOQ' property";
    koq = structArrayProp->GetKindOfQuantity();
    ASSERT_NE(nullptr, koq) << "'StructArrayWithKOQ' property does not have a KOQ as expected";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenKindOfQuantityInherited)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='A' modifier='abstract'>
                <ECProperty propertyName='KindOfQuantityProperty' typeName='double' kindOfQuantity='MyKindOfQuantity' />
            </ECEntityClass>
            <ECEntityClass typeName='B' modifer='abstract'>
                <BaseClass>A</BaseClass>
                <ECProperty propertyName='KindOfQuantityProperty' typeName='double' />
            </ECEntityClass>
            <ECEntityClass typeName='C' modifer='sealed'>
                <BaseClass>B</BaseClass>
                <ECProperty propertyName='KindOfQuantityProperty' typeName='double' />
            </ECEntityClass>
            <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here'
                displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3'
                presentationUnits='FT;IN;MILLIINCH'/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    for (auto const& pClass : schema->GetClasses())
        {
        ECPropertyP p = pClass->GetPropertyP("KindOfQuantityProperty");
        EXPECT_TRUE(p != nullptr);

        PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
        EXPECT_TRUE(prim != nullptr);

        KindOfQuantityCP kindOfQuantity = prim->GetKindOfQuantity();
        EXPECT_TRUE(kindOfQuantity != nullptr);

        if (pClass->GetName() != "A")
            EXPECT_FALSE(prim->IsKindOfQuantityDefinedLocally()) << "The Kind of Quantity is defined in a base class, so it should not be defined locally.";
        else
            EXPECT_TRUE(prim->IsKindOfQuantityDefinedLocally()) << "The Kind of Quantity is defined, so it should be defined locally.";

        EXPECT_STREQ(kindOfQuantity->GetName().c_str(), "MyKindOfQuantity");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithKindOfQuantityInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"KindOfQuantityInReferencedSchema.01.00.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP("Entity");
    ASSERT_TRUE(nullptr != pClass);

    ECPropertyP p = pClass->GetPropertyP("KindOfQuantityProperty");
    ASSERT_TRUE(p != nullptr);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_TRUE(prim != nullptr);

    KindOfQuantityCP kindOfQuantity = prim->GetKindOfQuantity();
    ASSERT_TRUE(kindOfQuantity != nullptr);

    EXPECT_STREQ(kindOfQuantity->GetName().c_str(), "MyKindOfQuantity");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenRoundtripKindOfQuantityUsingString)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));

    KindOfQuantityP kindOfQuantity;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("DESC");
    kindOfQuantity->SetDisplayLabel("DL");
    kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"));
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("Feet4U"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));

    ECEntityClassP entityClass;
    ASSERT_TRUE(schema->CreateEntityClass(entityClass, "EntityClass") == ECObjectsStatus::Success);
    PrimitiveArrayECPropertyP property;
    auto status = entityClass->CreatePrimitiveArrayProperty(property, "QuantifiedProperty", PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(property != nullptr);
    property->SetKindOfQuantity(kindOfQuantity);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::Latest);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    auto status3 = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status3);

    EXPECT_EQ(1, deserializedSchema->GetKindOfQuantityCount());
    KindOfQuantityCP deserializedKindOfQuantity;
    deserializedKindOfQuantity = deserializedSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(deserializedKindOfQuantity != nullptr);
    EXPECT_STREQ("DL", deserializedKindOfQuantity->GetDisplayLabel().c_str());
    EXPECT_STREQ("DESC", deserializedKindOfQuantity->GetDescription().c_str());
    EXPECT_STREQ("CM", deserializedKindOfQuantity->GetPersistenceUnit()->GetName().c_str());
    EXPECT_EQ(10e-3, deserializedKindOfQuantity->GetRelativeError());

    EXPECT_STREQ("f:Feet4U", deserializedKindOfQuantity->GetDefaultPresentationFormat()->GetName().c_str());
    auto& resultAltUnits = deserializedKindOfQuantity->GetPresentationFormatList();
    EXPECT_EQ(3, resultAltUnits.size()); // Default presentation unit is included in list of presentation units
    EXPECT_STREQ("f:InchesU", resultAltUnits[1].GetName().c_str());
    EXPECT_STREQ("f:AmerFI", resultAltUnits[2].GetName().c_str());

    ECClassCP deserializedClass = deserializedSchema->GetClassCP("EntityClass");
    ECPropertyP deserializedProperty = deserializedClass->GetPropertyP("QuantifiedProperty");
    KindOfQuantityCP propertyKoq = deserializedProperty->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
    ASSERT_TRUE(nullptr != propertyKoq);
    EXPECT_EQ(propertyKoq, deserializedKindOfQuantity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPersistenceUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                           <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                               displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown persistence unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPersistenceFormat)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                            <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                                <ECSchemaReference name="Units" version="01.00" alias="u"/>
                                                <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                   displayLabel="best quantity of all time" persistenceUnit="u:M(SILLYFORMAT)" relativeError="10e-3" />
                                            </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown persistence format");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPresentationUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00" alias="u"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                               displayLabel="best quantity of all time" persistenceUnit="u:M" presentationFormats="SILLYMETER;ANOTHERSILLYMETER" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an known presentation Unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPresentationFormat)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00" alias="u"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationFormats="u:MM;u:CM(SILLYFORMAT)" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown presentation format");
    }

//=======================================================================================
//! KindOfQuantitySerializationTest
//=======================================================================================

Utf8CP KindOfQuantitySerializationTest::s_testSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Units" version="01.00" alias="u"/>
        <ECSchemaReference name="Formats" version="01.00" alias="f"/>
        <KindOfQuantity typeName="TestKoQ" relativeError="10e-3" persistenceUnit="u:M" presentationFormats="f:InchesU" />
    </ECSchema>)xml";

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(KindOfQuantitySerializationTest, WriteXmlUsesProperUnitNameMappings)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(testSchema, s_testSchemaXml, *context)) << "Failed to deserialized initial base test schema.";

    Utf8String serializedSchemaXml;
    // EC3.0
    {
    serializedSchemaXml.clear();
    ASSERT_EQ(SchemaWriteStatus::Success, testSchema->WriteToXmlString(serializedSchemaXml, ECVersion::V3_0));

    ECSchemaPtr secondSchema;
    ECSchemaReadContextPtr secondContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(secondSchema, serializedSchemaXml.c_str(), *secondContext));
    EXPECT_EQ(2, secondSchema->GetReferencedSchemas().size());
    auto koq = secondSchema->GetKindOfQuantityCP("TestKoQ");
    auto persist = koq->GetPersistenceUnit();
    auto pres = koq->GetDefaultPresentationFormat();
    EXPECT_STRCASEEQ("M", persist->GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", pres->GetName().c_str());
    }

    // EC3.1
    {
    Utf8String serializedSchemaXml;
    ASSERT_EQ(SchemaWriteStatus::Success, testSchema->WriteToXmlString(serializedSchemaXml, ECVersion::V3_1));

    ECSchemaPtr secondSchema;
    ECSchemaReadContextPtr secondContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(secondSchema, serializedSchemaXml.c_str(), *secondContext));
    EXPECT_EQ(2, secondSchema->GetReferencedSchemas().size());
    auto koq = secondSchema->GetKindOfQuantityCP("TestKoQ");
    auto persist = koq->GetPersistenceUnit();
    auto pres = koq->GetDefaultPresentationFormat();
    EXPECT_STRCASEEQ("M", persist->GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", pres->GetName().c_str());
    }

    // EC3.2
    {

    Utf8String outSchemaXmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, testSchema->WriteToXmlString(outSchemaXmlString, ECVersion::V3_2));

    ECSchemaPtr secondSchema;
    ECSchemaReadContextPtr secondContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(secondSchema, outSchemaXmlString.c_str(), *secondContext));
    auto koq = secondSchema->GetKindOfQuantityCP("TestKoQ");
    auto persist = koq->GetPersistenceUnit();
    auto pres = koq->GetDefaultPresentationFormat();
    EXPECT_STRCASEEQ("M", persist->GetName().c_str());
    ASSERT_NE(nullptr, persist);
    EXPECT_STRCASEEQ("f:InchesU", pres->GetName().c_str());
    }

    // Latest EC Version
    {
    ECSchemaPtr origSchema;
    ECSchema::CreateSchema(origSchema, "ExampleSchema", "ex", 5, 0, 5, ECVersion::Latest);
    EC_EXPECT_SUCCESS(origSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(origSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));
    KindOfQuantityP koq;
    EC_ASSERT_SUCCESS(origSchema->CreateKindOfQuantity(koq, "ExampleKoQ"));
    EXPECT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EXPECT_EQ(ECObjectsStatus::Success, koq->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU")));

    Utf8String xmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, origSchema->WriteToXmlString(xmlString, ECVersion::Latest));

    ECSchemaPtr readSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(readSchema, xmlString.c_str(), *context));
    auto koq2 = readSchema->GetKindOfQuantityCP("ExampleKoQ");
    auto persist = koq2->GetPersistenceUnit();
    auto pres = koq2->GetDefaultPresentationFormat();
    EXPECT_STRCASEEQ("M", persist->GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", pres->GetName().c_str());
    EXPECT_STRCASEEQ("IN", pres->GetCompositeMajorUnit()->GetName().c_str());
    }
    }

//=======================================================================================
//! KindOfQuantityRoundTripTest
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, Fail_ec31_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"
                        presentationFormats="f:InchesU(9)[u:IN|banana];f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchemaToVersionAndBack(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::InvalidECSchemaXml, SchemaWriteStatus::FailedToSaveXml, "Should fail to round trip a 3.1 schema with KoQ using EC3.2 unit defined in schema as persistence unit");
    ASSERT_FALSE(schema.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec31_roundTripShouldDropUnknownPresentationFormats)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationFormats="f:InchesU(9)[u:IN|banana];f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchemaToVersionAndBack(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should succeed to round trip a 3.1 schema with KoQ using EC3.2 unit defined in schema as presentation unit");
    ASSERT_TRUE(schema.IsValid());
    ASSERT_EQ(1, schema->GetKindOfQuantityCP("MyKindOfQuantity")->GetPresentationFormatList().size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, Fail_ec30_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"
                        presentationFormats="f:InchesU(9)[u:IN|banana];f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchemaToVersionAndBack(schema, schemaItem, ECVersion::V3_0, SchemaReadStatus::InvalidECSchemaXml, SchemaWriteStatus::FailedToSaveXml, "Should fail to round trip a 3.0 schema with KoQ using EC3.2 unit defined in schema as persistence unit");
    ASSERT_FALSE(schema.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec30_roundTripShouldDropUnknownPresentationFormats)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationFormats="f:InchesU(9)[u:IN|banana];f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchemaToVersionAndBack(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should succeed to round trip a 3.0 schema with KoQ using EC3.2 unit defined in schema as presentation unit");
    ASSERT_TRUE(schema.IsValid());
    ASSERT_EQ(1, schema->GetKindOfQuantityCP("MyKindOfQuantity")->GetPresentationFormatList().size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec31_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationFormats="f:AmerFI;f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchemaToVersionAndBack(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should be able to round trip a schema from 3.2 -> 3.1 -> 3.2");
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_EQ(2, koq->GetPresentationFormatList().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormatList()[0].GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", koq->GetPresentationFormatList()[1].GetName().c_str());
    EXPECT_DOUBLE_EQ(0.5, koq->GetRelativeError());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec30_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationFormats="f:AmerFI;f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchemaToVersionAndBack(schema, schemaItem, ECVersion::V3_0,  SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should be able to round trip a schema from 3.2 -> 3.0 -> 3.2");
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_EQ(2, koq->GetPresentationFormatList().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormatList()[0].GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", koq->GetPresentationFormatList()[1].GetName().c_str());
    EXPECT_DOUBLE_EQ(0.5, koq->GetRelativeError());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    }

//=======================================================================================
//! KindOfQuantityUpgradeTest
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_ValidKindOfQuantityInReferencedSchema)
    {
    SchemaItem refSchemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="CM" relativeError=".5"
                        presentationUnits="FT;IN" />
        </ECSchema>)xml");

    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="Schema1" version="01.00.00" alias="s1" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="s1:MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="s1:MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    DeserializeSchema(refSchema, *schemaContext, refSchemaItem);
    ASSERT_TRUE(refSchema.IsValid());
    ASSERT_TRUE(refSchema->Validate());
    ECSchemaPtr schema;
    DeserializeSchema(schema, *schemaContext, schemaItem);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());

    auto koq = refSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    EXPECT_STRCASEEQ("CM", koq->GetPersistenceUnit()->GetName().c_str());
    ASSERT_EQ(2, koq->GetPresentationFormatList().size());
    EXPECT_STRCASEEQ("f:DefaultRealU[u:FT]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("f:DefaultRealU[u:IN]", koq->GetPresentationFormatList()[1].GetName().c_str());
    auto entityClass = schema->GetClassCP("Foo");
    auto propWithKoq = entityClass->GetPropertyP("Length");
    auto arrayPropWithKoq = entityClass->GetPropertyP("AlternativeLengths");
    auto propKoq = propWithKoq->GetKindOfQuantity();
    auto arrayPropKoq = arrayPropWithKoq->GetKindOfQuantity();
    ASSERT_EQ(propKoq, koq);
    ASSERT_EQ(propKoq, arrayPropKoq);
    }

//=======================================================================================
//! KindOfQuantityCompatibilityTest
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCompatibilityTest, Fail_UnknownUnit)
    {
    // Persistence FUS
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <KindOfQuantity typeName="KoQWithPers" description="Kind of a Description here"
                displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an unknown perisistence unit in a newer (3.3) version");
    // Presentation FUS
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <KindOfQuantity typeName="KoQWithPres" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="u:M" presentationFormats="f:InchesU;ANOTHERSILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an unknown presentation format in a newer (3.3) version");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCompatibilityTest, Fail_UnknownFormat)
    {
    // Presentation Units
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <KindOfQuantity typeName="KoQWithPres" description="Kind of a Description here"
                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationFormats="f:InchesU;SILLYFORMAT" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid presentation format in EC 3.3");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCompatibilityTest, ec33_ValidKindOfQuantityInReferencedSchema)
    {
    SchemaItem refSchemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5"
                        presentationFormats="f:AmerFI;f:InchesU" />
        </ECSchema>)xml");

    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Schema1" version="01.00.00" alias="s1" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="s1:MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="s1:MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    DeserializeSchema(refSchema, *schemaContext, refSchemaItem);
    ASSERT_TRUE(refSchema.IsValid());
    ASSERT_TRUE(refSchema->Validate());
    ECSchemaPtr schema;
    DeserializeSchema(schema, *schemaContext, schemaItem);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());

    auto koq = refSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    EXPECT_STRCASEEQ("CM", koq->GetPersistenceUnit()->GetName().c_str());
    ASSERT_EQ(2, koq->GetPresentationFormatList().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", koq->GetPresentationFormatList()[1].GetName().c_str());
    auto entityClass = schema->GetClassCP("Foo");
    auto propWithKoq = entityClass->GetPropertyP("Length");
    auto arrayPropWithKoq = entityClass->GetPropertyP("AlternativeLengths");
    auto propKoq = propWithKoq->GetKindOfQuantity();
    auto arrayPropKoq = arrayPropWithKoq->GetKindOfQuantity();
    ASSERT_EQ(propKoq, koq);
    ASSERT_EQ(propKoq, arrayPropKoq);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCompatibilityTest, ec33_ValidKindOfQuantityInSchema)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5"
                        presentationFormats="f:InchesU;f:Feet4U" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *schemaContext, schemaItem);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());

    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    EXPECT_STRCASEEQ("CM", koq->GetPersistenceUnit()->GetName().c_str());
    ASSERT_EQ(2, koq->GetPresentationFormatList().size());
    EXPECT_STRCASEEQ("f:InchesU", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("f:Feet4U", koq->GetPresentationFormatList()[1].GetName().c_str());
    auto entityClass = schema->GetClassCP("Foo");
    auto propWithKoq = entityClass->GetPropertyP("Length");
    auto arrayPropWithKoq = entityClass->GetPropertyP("AlternativeLengths");
    auto propKoq = propWithKoq->GetKindOfQuantity();
    auto arrayPropKoq = arrayPropWithKoq->GetKindOfQuantity();
    ASSERT_EQ(propKoq, koq);
    ASSERT_EQ(propKoq, arrayPropKoq);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCompatibilityTest, ec33_ValidKindOfQuantityWithUnitsInSchema)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"
                        presentationFormats="f:DefaultRealU[TestUnit];f:AmerFI;f:InchesU" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *schemaContext, schemaItem);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());

    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    EXPECT_STRCASEEQ("TestUnit", koq->GetPersistenceUnit()->GetName().c_str());
    ASSERT_EQ(3, koq->GetPresentationFormatList().size());
    EXPECT_STRCASEEQ("f:DefaultRealU[TestUnit]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormatList()[1].GetName().c_str());
    EXPECT_STRCASEEQ("f:InchesU", koq->GetPresentationFormatList()[2].GetName().c_str());
    auto entityClass = schema->GetClassCP("Foo");
    auto propWithKoq = entityClass->GetPropertyP("Length");
    auto arrayPropWithKoq = entityClass->GetPropertyP("AlternativeLengths");
    auto propKoq = propWithKoq->GetKindOfQuantity();
    auto arrayPropKoq = arrayPropWithKoq->GetKindOfQuantity();
    ASSERT_EQ(propKoq, koq);
    ASSERT_EQ(propKoq, arrayPropKoq);
    EXPECT_EQ(schema->GetUnitCP("TestUnit"), koq->GetPersistenceUnit());
    EXPECT_EQ(schema->GetUnitCP("TestUnit"), koq->GetDefaultPresentationFormat()->GetCompositeMajorUnit());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
