/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
            EC_EXPECT_SUCCESS(m_schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
        }
    };
struct KindOfQuantityRoundTripTest : ECTestFixture {};
struct KindOfQuantityUpgradeTest : ECTestFixture
    {
    static void VerifySchemaReferencesUnitsSchema(ECSchemaPtr);
    static void VerifySchemaReferencesFormatsSchema(ECSchemaPtr);
    static bool SchemaReferencesUnitsSchema(ECSchemaPtr);
    static bool SchemaReferencesFormatsSchema(ECSchemaPtr);
    };
struct KindOfQuantityCompatibilityTest : ECTestFixture {};
struct KindOfQuantityDeserializationTest : ECTestFixture {};
struct KindOfQuantitySerializationTest : ECTestFixture 
    {
    static Utf8CP s_testSchemaXml;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Basanta.Kharel   12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(KindOfQuantityTest, CreationTest)
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
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));
    {
    KindOfQuantityP kindOfQuantity;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    EC_EXPECT_SUCCESS(kindOfQuantity->SetDescription("Kind of a Description here"));
    EC_EXPECT_SUCCESS(kindOfQuantity->SetDisplayLabel("best quantity of all times"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    EXPECT_NE(ECObjectsStatus::Success, kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetConstantCP("PI"))); // constant
    kindOfQuantity->SetRelativeError(10e-3);
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EC_EXPECT_SUCCESS(kindOfQuantity->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultReal"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EXPECT_NE(ECObjectsStatus::Success, kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("AngleDMS")));
    }

    {
    KindOfQuantityP koq = schema->GetKindOfQuantityP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STREQ("DefaultRealU[u:M]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    auto const& presUnitList = koq->GetPresentationFormats();

    EXPECT_EQ(2, presUnitList.size());
    EXPECT_STREQ("DefaultRealU[u:M]", presUnitList.at(0).GetName().c_str());
    EXPECT_STREQ("DefaultReal[u:M]", presUnitList.at(1).GetName().c_str());
    }
    
    {
    KindOfQuantityP koq = schema->GetKindOfQuantityP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    auto const& presUnitList = koq->GetPresentationFormats();

    koq->RemovePresentationFormat(presUnitList.at(1));
    EXPECT_EQ(1, presUnitList.size());
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", presUnitList.at(0).GetName().c_str());

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

    EXPECT_EQ(0, kindOfQuantity->GetPresentationFormats().size());

    EC_EXPECT_SUCCESS(kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EXPECT_EQ(1, kindOfQuantity->GetPresentationFormats().size());
    kindOfQuantity->RemoveAllPresentationFormats();
    EXPECT_EQ(0, kindOfQuantity->GetPresentationFormats().size());

    EC_EXPECT_SUCCESS(kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EC_EXPECT_SUCCESS(kindOfQuantity->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultReal"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EXPECT_EQ(2, kindOfQuantity->GetPresentationFormats().size());

    kindOfQuantity->RemoveAllPresentationFormats();
    EXPECT_EQ(0, kindOfQuantity->GetPresentationFormats().size());
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
    ECUnitCP arcDegUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP("ARC_DEG");
    ECFormatCP angleDm = ECTestFixture::GetFormatsSchema()->GetFormatCP("AngleDMS");
    ECFormatCP amerFI = ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI");
    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    EC_EXPECT_SUCCESS(koq->SetPersistenceUnit(*meterUnit));
    EC_EXPECT_SUCCESS(koq->AddPresentationFormat(*amerFI));
    EXPECT_NE(ECObjectsStatus::Success, koq->AddPresentationFormat(*angleDm)) << "The input unit ARC_DEG is from a different phenomeonon than the Persistence Unit.";
    EXPECT_NE(ECObjectsStatus::Success, koq->SetPersistenceUnit(*arcDegUnit)) << "The input unit ARC_DEG is from a different phenomeonon than the current presentation units";
    EXPECT_EQ(1, koq->GetPresentationFormats().size());

    KindOfQuantityP koq2;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq2, "MyKindOfQuantity3"));
    EC_EXPECT_SUCCESS(koq2->SetPersistenceUnit(*centimeterUnit));
    EXPECT_NE(ECObjectsStatus::Success, koq2->AddPresentationFormat(*angleDm)) << "The input unit ARC_DEG is from a different phenomeonon than the Persistence Unit and presentation unit.";
    EC_EXPECT_SUCCESS(koq2->SetPersistenceUnit(*arcDegUnit));
    EC_EXPECT_SUCCESS(koq2->AddPresentationFormat(*angleDm));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, UpdateFUSDescriptor)
    {
    Utf8String persUnitName;
    bvector<Utf8String> presFormatStrings;
    bvector<Utf8CP> presFUSes;
    auto& schema = *GetFormatsSchema();
    auto& unitsSchema = *GetUnitsSchema();
    {
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, nullptr, presFUSes, schema, unitsSchema));
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, nullptr, presFUSes, schema, unitsSchema));
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "", presFUSes, schema, unitsSchema));
    EXPECT_TRUE(persUnitName.empty());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM", presFUSes, schema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': LUX
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("LUX");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM", presFUSes, schema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: badUnit
    // old presentationFUS': -
    // new persistenceUnit: -
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EXPECT_EQ(ECObjectsStatus::InvalidUnitName, KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "badUnit", presFUSes, schema, unitsSchema))
        << "Should fail if the persistenceFUS has an invalid Unit.";
    EXPECT_TRUE(persUnitName.empty());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: MM(SillyFormat)
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(SillyFormat)", presFUSes, schema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: DM(fi8)
    // old presentationFUS': -
    // new persistenceUnit: u:DM
    // new presentationFormats: f:AmerFI
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "DM(fi8)", presFUSes, schema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:DM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:AmerFI", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: MM(real)
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: f:defaultReal[u:MM]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(real)", presFUSes, schema, unitsSchema))
        << "Should succeed if persistenceFUS has a valid unit and format";
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:MM]", presFormatStrings[0].c_str()) << "A presentation format should have been created from the persistenceFUS' format.";
    }
    {
    // old persistenceFUS: W/(M*K)
    // old presentationFUS': -
    // new persistenceUnit: u:W_PER_M_K
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "W/(M*K)", presFUSes, schema, unitsSchema))
        << "Should succeed if persistenceFUS has a valid unit and format";
    EXPECT_STRCASEEQ("u:W_PER_M_K", persUnitName.c_str());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': badUnit
    // new persistenceUnit: -
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("badUnit");
    EXPECT_EQ(ECObjectsStatus::InvalidUnitName, KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM", presFUSes, schema, unitsSchema))
        << "Should fail if a presentation FUS has an invalid Unit";
    EXPECT_TRUE(persUnitName.empty());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': MM(KindOfFormat)
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("MM(KindOfFormat)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM", presFUSes, schema, unitsSchema))
        << "Should drop a presentation FUS if it has an invalid Format.";
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: MM(KindOfFormat)
    // old presentationFUS': MM
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultReal[u:MM]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("MM");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(KindOfFormat)", presFUSes, schema, unitsSchema))
        << "Should drop the format from the persistence FUS if it cannot be found.";
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:MM]", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: MM(realu)
    // old presentationFUS': CM
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultReal[u:CM]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("CM");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(realu)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:CM]", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: MM(real)
    // old presentationFUS': CM(real4u)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultRealU(4)[u:CM]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("CM(real4u)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(real)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM]", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': CM(real4u)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultRealU(4)[u:CM]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("CM(real4u)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM]", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: MM(real)
    // old presentationFUS': DM(real);CM(real4u)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultRealU[u:DM];f:DefaultRealU(4)[u:CM]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("DM(real)");
    presFUSes.push_back("CM(real4u)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(real)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(2, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:DM]", presFormatStrings[0].c_str());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM]", presFormatStrings[1].c_str());
    }
    {
    // old persistenceFUS: MM(fi8)
    // old presentationFUS': DM(fi8)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:AmerFI
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("DM(fi8)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(fi8)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:AmerFI", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: MM(fi8)
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: f:AmerFI
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "MM(fi8)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:AmerFI", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: SQ.FT(fi8)
    // old presentationFUS': -
    // new persistenceUnit: u:SQ_FT
    // new presentationFormats: -
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "SQ.FT(fi8)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:SQ_FT", persUnitName.c_str());
    EXPECT_TRUE(presFormatStrings.empty());
    }
    {
    // old persistenceFUS: M(meters4u)
    // old presentationFUS': IN(inches4u)
    // new persistenceUnit: u:M
    // new presentationFormats: f:DefaultRealUNS(4)[u:M|m]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "M(meters4u)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:M", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultRealUNS(4)[u:M|m]", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: M(meters4u)
    // old presentationFUS': IN(inches4u)
    // new persistenceUnit: u:M
    // new presentationFormats: f:DefaultRealUNS(4)[u:IN|&quot;]
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("IN(inches4u)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "M(meters4u)", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:M", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:DefaultRealUNS(4)[u:IN|&quot;]", presFormatStrings[0].c_str());
    }
    {
    // old persistenceFUS: IN
    // old presentationFUS': IN(fi8)
    // new persistenceUnit: u:IN
    // new presentationFormats: f:AmerFI
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    presFUSes.push_back("IN(fi8)");
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "IN", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:IN", persUnitName.c_str());
    EXPECT_EQ(1, presFormatStrings.size());
    EXPECT_STRCASEEQ("f:AmerFI", presFormatStrings[0].c_str());
    }
    {
    persUnitName.clear();
    presFormatStrings.clear();
    presFUSes.clear();
    EC_EXPECT_SUCCESS(KindOfQuantity::UpdateFUSDescriptors(persUnitName, presFormatStrings, "IN", presFUSes, schema, unitsSchema));
    EXPECT_STRCASEEQ("u:IN", persUnitName.c_str());
    EXPECT_EQ(0, presFormatStrings.size());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Joseph.Urbano    09/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, VerifyDescriptorCacheIsAccurate)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    EC_EXPECT_SUCCESS(koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));

    auto cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("M", cache.first.c_str());

    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    EXPECT_TRUE(cache.second.empty());

    // change persistence unit
    EC_EXPECT_SUCCESS(koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));

    cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("CM", cache.first.c_str());

    // add presentation formats
    EC_EXPECT_SUCCESS(koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultReal"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    EC_EXPECT_SUCCESS(koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultReal"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("MM")));

    cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("CM", cache.first.c_str());
    EXPECT_EQ(2, koq->GetPresentationFormats().size());
    ASSERT_EQ(2, cache.second.size());

    EXPECT_STRCASEEQ("CM(real)", cache.second[0].c_str());
    EXPECT_STRCASEEQ("MM(real)", cache.second[1].c_str());

    // add default presentation format
    EC_EXPECT_SUCCESS(koq->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->LookupFormat("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));

    cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("CM", cache.first.c_str());
    EXPECT_EQ(3, koq->GetPresentationFormats().size());
    ASSERT_EQ(3, cache.second.size());

    EXPECT_STRCASEEQ("M(realu)", cache.second[0].c_str());
    EXPECT_STRCASEEQ("CM(real)", cache.second[1].c_str());
    EXPECT_STRCASEEQ("MM(real)", cache.second[2].c_str());

    // remove presentation format
    koq->RemovePresentationFormat(koq->GetPresentationFormats().at(1));

    cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("CM", cache.first.c_str());
    EXPECT_EQ(2, koq->GetPresentationFormats().size());
    ASSERT_EQ(2, cache.second.size());

    EXPECT_STRCASEEQ("M(realu)", cache.second[0].c_str());
    EXPECT_STRCASEEQ("MM(real)", cache.second[1].c_str());

    // remove all presentation formats
    koq->RemoveAllPresentationFormats();

    cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("CM", cache.first.c_str());

    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    ASSERT_TRUE(cache.second.empty());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, FromFUSDescriptor)
    {
    bvector<Utf8CP> presFUSes;
    auto& formatsSchema = *GetFormatsSchema();
    auto& unitsSchema = *GetUnitsSchema();
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, koq->FromFUSDescriptors(nullptr, presFUSes, formatsSchema, unitsSchema));
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, koq->FromFUSDescriptors("", presFUSes, formatsSchema, unitsSchema));
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM", presFUSes, formatsSchema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': LUX
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("LUX");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM", presFUSes, formatsSchema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("LUX", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: badUnit
    // old presentationFUS': -
    // new persistenceUnit: -
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EXPECT_EQ(ECObjectsStatus::InvalidUnitName, koq->FromFUSDescriptors("badUnit", presFUSes, formatsSchema, unitsSchema))
        << "Should fail if the persistenceFUS has an invalid Unit.";
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }
    {
    // old persistenceFUS: MM(SillyFormat)
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(SillyFormat)", presFUSes, formatsSchema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(SillyFormat)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: DM(fi8)
    // old presentationFUS': -
    // new persistenceUnit: u:DM
    // new presentationFormats: f:AmerFI
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("DM(fi8)", presFUSes, formatsSchema, unitsSchema))
        << "Should succeed if the persistenceFUS has a valid unit.";
    EXPECT_STRCASEEQ("u:DM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("DM(fi8)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: MM(real)
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: f:defaultReal[u:MM]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(real)", presFUSes, formatsSchema, unitsSchema))
        << "Should succeed if persistenceFUS has a valid unit and format";
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:MM]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str()) << "A presentation format should have been created from the persistenceFUS' format.";
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(real)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: W/(M*K)
    // old presentationFUS': -
    // new persistenceUnit: u:W_PER_M_K
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("W/(M*K)", presFUSes, formatsSchema, unitsSchema))
        << "Should succeed if persistenceFUS has a valid unit and format";
    EXPECT_STRCASEEQ("u:W_PER_M_K", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("W/(M*K)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': badUnit
    // new persistenceUnit: -
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("badUnit");
    EXPECT_EQ(ECObjectsStatus::InvalidUnitName, koq->FromFUSDescriptors("MM", presFUSes, formatsSchema, unitsSchema))
        << "Should fail if a presentation FUS has an invalid Unit";
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': MM(KindOfFormat)
    // new persistenceUnit: u:MM
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("MM(KindOfFormat)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM", presFUSes, formatsSchema, unitsSchema))
        << "Should drop a presentation FUS if it has an invalid Format.";
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("MM(KindOfFormat)", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: MM(KindOfFormat)
    // old presentationFUS': MM
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultReal[u:MM]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("MM");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(KindOfFormat)", presFUSes, formatsSchema, unitsSchema))
        << "Should drop the format from the persistence FUS if it cannot be found.";
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:MM]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(KindOfFormat)", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("MM", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: MM(realu)
    // old presentationFUS': CM
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultReal[u:CM]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("CM");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(realu)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:CM]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(realu)", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("CM", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: MM(real)
    // old presentationFUS': CM(real4u)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultRealU(4)[u:CM]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("CM(real4u)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(real)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(real)", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("CM(real4u)", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: MM
    // old presentationFUS': CM(real4u)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultRealU(4)[u:CM]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("CM(real4u)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("CM(real4u)", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: MM(real)
    // old presentationFUS': DM(real);CM(real4u)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:DefaultRealU[u:DM];f:DefaultRealU(4)[u:CM]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("DM(real)");
    presFUSes.push_back("CM(real4u)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(real)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultReal[u:DM]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:CM]", koq->GetPresentationFormats()[1].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(real)", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(2, cache.second.size());
    EXPECT_STRCASEEQ("DM(real)", cache.second.front().c_str());
    EXPECT_STRCASEEQ("CM(real4u)", cache.second[1].c_str());
    }
    {
    // old persistenceFUS: MM(fi8)
    // old presentationFUS': DM(fi8)
    // new persistenceUnit: u:MM
    // new presentationFormats: f:AmerFI
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("DM(fi8)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(fi8)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(fi8)", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("DM(fi8)", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: MM(fi8)
    // old presentationFUS': -
    // new persistenceUnit: u:MM
    // new presentationFormats: f:AmerFI
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("MM(fi8)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:MM", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("MM(fi8)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: SQ.FT(fi8)
    // old presentationFUS': -
    // new persistenceUnit: u:SQ_FT
    // new presentationFormats: -
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("SQ.FT(fi8)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:SQ_FT", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("SQ.FT(fi8)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: M(meters4u)
    // old presentationFUS': IN(inches4u)
    // new persistenceUnit: u:M
    // new presentationFormats: f:DefaultRealUNS(4)[u:M|m]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("M(meters4u)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:M", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultRealUNS(4)[u:M|m]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("M(meters4u)", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    {
    // old persistenceFUS: M(meters4u)
    // old presentationFUS': IN(inches4u)
    // new persistenceUnit: u:M
    // new presentationFormats: f:DefaultRealUNS(4)[u:IN|&quot;]
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("IN(inches4u)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("M(meters4u)", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:M", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:DefaultRealUNS(4)[u:IN|&quot;]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("M(meters4u)", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("IN(inches4u)", cache.second.front().c_str());
    }
    {
    // old persistenceFUS: IN
    // old presentationFUS': IN(fi8)
    // new persistenceUnit: u:IN
    // new presentationFormats: f:AmerFI
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    presFUSes.push_back("IN(fi8)");
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("IN", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:IN", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("f:AmerFI", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*schema).c_str());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("IN", cache.first.c_str());
    EXPECT_FALSE(cache.second.empty());
    EXPECT_EQ(1, cache.second.size());
    EXPECT_STRCASEEQ("IN(fi8)", cache.second.front().c_str());
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "test", "t", 1, 0, 1);
    schema->AddReferencedSchema(formatsSchema);
    schema->AddReferencedSchema(unitsSchema);
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKoq");
    presFUSes.clear();
    EC_EXPECT_SUCCESS(koq->FromFUSDescriptors("IN", presFUSes, formatsSchema, unitsSchema));
    EXPECT_STRCASEEQ("u:IN", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
    EXPECT_EQ(0, koq->GetPresentationFormats().size());
    const auto& cache = koq->GetDescriptorCache();
    EXPECT_STRCASEEQ("IN", cache.first.c_str());
    EXPECT_TRUE(cache.second.empty());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, AddPersistenceUnitByNameTest)
    {
    KindOfQuantityP koq;
    const auto unitLookerUpper = [&] (Utf8StringCR alias, Utf8StringCR name)
        {
        return koq->GetSchema().GetUnitsContext().LookupUnit((alias + ":" + name).c_str());
        };

    const auto formatLookerUpper = [&] (Utf8StringCR alias, Utf8String name)
        {
        return koq->GetSchema().LookupFormat((alias + ":" + name).c_str());
        };
    
    {
    CreateTestSchema(true);
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    koq->AddPersistenceUnitByName("u:M", unitLookerUpper);
    EXPECT_STRCASEEQ("M", koq->GetPersistenceUnit()->GetName().c_str());
    }
    
    {
    CreateTestSchema(true);
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    koq->AddPersistenceUnitByName("u:fakeUnit", unitLookerUpper);
    EXPECT_EQ(nullptr, koq->GetPersistenceUnit());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityTest, AddPresentationFormatByString)
    {
    KindOfQuantityP koq;

    const auto unitLookerUpper = [&] (Utf8StringCR alias, Utf8StringCR name)
        {
        return koq->GetSchema().GetUnitsContext().LookupUnit((alias + ":" + name).c_str());
        };

    const auto formatLookerUpper = [&] (Utf8StringCR alias, Utf8String name)
        {
        return koq->GetSchema().LookupFormat((alias + ":" + name).c_str());
        };

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    koq->SetPersistenceUnit(*m_schema->GetUnitsContext().LookupUnit("u:M"));
    EXPECT_NE(nullptr, koq->GetDefaultPresentationFormat());
    // Cannot add a format with no units without specifying unit overrides
    EXPECT_EQ(ECObjectsStatus::Error, koq->AddPresentationFormatByString("f:DefaultRealU", formatLookerUpper, unitLookerUpper));
    }

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    koq->SetPersistenceUnit(*m_schema->GetUnitsContext().LookupUnit("u:M"));
    EXPECT_NE(nullptr, koq->GetDefaultPresentationFormat());
    // Cannot add a format with a composite without specifying all units defined in the overrides
    EXPECT_EQ(ECObjectsStatus::Error, koq->AddPresentationFormatByString("f:AmerFI[u:FT]", formatLookerUpper, unitLookerUpper));
    }

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    koq->SetPersistenceUnit(*m_schema->GetUnitsContext().LookupUnit("u:M"));
    EXPECT_NE(nullptr, koq->GetDefaultPresentationFormat());
    koq->AddPresentationFormatByString("f:DefaultRealU[u:M]", formatLookerUpper, unitLookerUpper);
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    }

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    koq->SetPersistenceUnit(*m_schema->GetUnitsContext().LookupUnit("u:M"));
    EXPECT_NE(nullptr, koq->GetDefaultPresentationFormat());
    koq->AddPresentationFormatByString("f:DefaultRealU(8)[u:M]", formatLookerUpper, unitLookerUpper);
    EXPECT_STRCASEEQ("DefaultRealU(8)[u:M]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    }

    {
    CreateTestSchema(true);
    m_schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    m_schema->CreateKindOfQuantity(koq, "KindOfAwesome");
    koq->SetPersistenceUnit(*m_schema->GetUnitsContext().LookupUnit("u:M"));
    EXPECT_NE(nullptr, koq->GetDefaultPresentationFormat());
    koq->AddPresentationFormatByString("f:DefaultRealU(8)[u:IN|inch(es)]", formatLookerUpper, unitLookerUpper);
    EXPECT_STRCASEEQ("DefaultRealU(8)[u:IN|inch(es)]", koq->GetDefaultPresentationFormat()->GetName().c_str());
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
    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), 3, ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    const std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> unitMapper = [&] (Utf8StringCR alias, Utf8StringCR name)
        {
        return schema->LookupUnit((alias + ":" + name).c_str());
        };
    const std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> formatMapper = [&] (Utf8StringCR alias, Utf8StringCR name)
        {
        return schema->LookupFormat((alias + ":" + name).c_str());
        };
    koq->AddPresentationFormatByString("f:DefaultRealU[u:MM|]", formatMapper, unitMapper);
    koq->AddPresentationFormatByString("f:DefaultReal[u:M|meters]", formatMapper, unitMapper);
    koq->AddPresentationFormatByString("f:AmerFI[u:FT|feets][u:IN|inches]", formatMapper, unitMapper);
    koq->SetRelativeError(3);

    Json::Value schemaJson;
    EXPECT_TRUE(koq->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneKindOfQuantity.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  05/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, LookupKindOfQuantityTest)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="Smoot" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="Smoot" relativeError="10e-3"/>
        </ECSchema>)xml";
    Utf8CP koqSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="koqSchema" alias="ks" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="testSchema" version="01.00.00" alias="ts"/>
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

    auto shouldBeNull = koqSchema->LookupKindOfQuantity("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = koqSchema->LookupKindOfQuantity("banana");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = koqSchema->LookupKindOfQuantity("banana:MyKindOfQuantity");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = koqSchema->LookupKindOfQuantity("koqSchema:MyKindOfQuantity");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = koqSchema->LookupKindOfQuantity("testSchema:MyKindOfQuantity");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = koqSchema->LookupKindOfQuantity("ts:MyKindOfQuantity", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = koqSchema->LookupKindOfQuantity("MyKindOfQuantity");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = koqSchema->LookupKindOfQuantity("ts:MyKindOfQuantity");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = koqSchema->LookupKindOfQuantity("testSchema:MyKindOfQuantity", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = koqSchema->LookupKindOfQuantity("koqSchema:MyKindOfQuantity", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = koqSchema->LookupKindOfQuantity("TS:MyKindOfQuantity");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = koqSchema->LookupKindOfQuantity("TESTSCHEMA:MyKindOfQuantity", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = koqSchema->LookupKindOfQuantity("KOQSCHEMA:MyKindOfQuantity", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("MyKindOfQuantity", shouldNotBeNull->GetName().c_str());
    ASSERT_EQ(1, koqSchema->GetKindOfQuantityCount());
    }

//=======================================================================================
//! KindOfQuantityDeserializationTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Invalid_EmptyOrMissingName)
    {
    // Missing name
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <KindOfQuantity description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="u:CM" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a missing typeName");
    // Empty name
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <KindOfQuantity typeName="" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="u:CM" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty typeName");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Invalid_EmptyOrMissingRelativeError)
    {
    // Missing relative error
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <KindOfQuantity typeName="TestKOQ" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="u:CM"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a missing relative error");
    // Empty relative error
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <KindOfQuantity typeName="TestKOQ" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="u:CM" relativeError=""/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty relative error");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Invalid_EmptyOrMissingPersistenceUnit)
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
TEST_F(KindOfQuantityDeserializationTest, Invalid_ConstantAsPersistanceUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all times" persistenceUnit="u:PI" relativeError="10e-3"/>
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a constant as a persistence unit.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, PersistenceUnitInSchema)
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
TEST_F(KindOfQuantityDeserializationTest, PersistenceUnitInReferencedSchema)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="Smoot" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP koqSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="koqSchema" alias="ks" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="testSchema" version="01.00.00" alias="ts"/>
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
TEST_F(KindOfQuantityDeserializationTest, IncompatiblePresentationUnits)
    {
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX" presentationUnits="M" />
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_TRUE(koq->GetPresentationFormats().empty());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="LUX"/>
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_TRUE(koq->GetPresentationFormats().empty());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="M(fi8)"/>
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_FALSE(koq->GetPresentationFormats().empty());
            EXPECT_EQ(1, koq->GetPresentationFormats().size());
            EXPECT_STRCASEEQ("f:AmerFI", koq->GetDefaultPresentationFormat()->GetQualifiedFormatString(*schema).c_str());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="SQ.FT(real4u)"/>
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_TRUE(koq->GetPresentationFormats().empty());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M(fi8)"/>
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_FALSE(koq->GetPresentationFormats().empty());
            EXPECT_EQ(1, koq->GetPresentationFormats().size());
            EXPECT_STRCASEEQ("f:AmerFI", koq->GetDefaultPresentationFormat()->GetQualifiedFormatString(*schema).c_str());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="SQ.FT(fi8)"/>
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_TRUE(koq->GetPresentationFormats().empty());
            EXPECT_STRCASEEQ("u:SQ_FT", koq->GetPersistenceUnit()->GetQualifiedName(*schema).c_str());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="SQ.FT(fi8)" presentationUnits="SQ.FT(real4u)"/>
        </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_FALSE(koq->GetPresentationFormats().empty());
            EXPECT_EQ(1, koq->GetPresentationFormats().size());
            EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:SQ_FT]", koq->GetDefaultPresentationFormat()->GetQualifiedFormatString(*schema).c_str());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="CM;LUX"/>
    </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_FALSE(koq->GetPresentationFormats().empty());
            EXPECT_EQ(1, koq->GetPresentationFormats().size());
            EXPECT_STRCASEEQ("f:DefaultReal[u:CM]", koq->GetDefaultPresentationFormat()->GetQualifiedFormatString(*schema).c_str());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX" presentationUnits="CM;MM"/>
    </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_TRUE(koq->GetPresentationFormats().empty());
            }
            {
            Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX(real4u)" presentationUnits="CM;MM"/>
    </ECSchema>)xml";
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_FALSE(koq->GetPresentationFormats().empty());
            EXPECT_EQ(1, koq->GetPresentationFormats().size());
            EXPECT_STRCASEEQ("f:DefaultRealU(4)[u:LUX]", koq->GetDefaultPresentationFormat()->GetQualifiedFormatString(*schema).c_str());
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
// TODO: Possibly move to PropertyTests?
TEST_F(KindOfQuantityDeserializationTest, KindOfQuantityIsAppliedToStructAndStructArrayPropertes)
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
// TODO: Possibly move to PropertyTests?
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
// TODO: Possibly move to PropertyTests? Not really a need to test with a file.
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
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPersistenceUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                           <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                               displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown persistence unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPresentationUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                               displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:banana]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown unit as a unit override.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_UnknownPresentationFormat)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" persistenceUnit="u:M" presentationUnits="banana" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown presentation format");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_ConstantAsUnitOverride)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:M]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:PI]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a constant as an override");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_NoUnitsInPresentationUnits)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:M]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a constant as an override");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_OverrideInconsistentWithPersistenceUnit)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:M]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:ACRE]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with override units that are inconsistent with pers unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_OverrideInconsistentWithOtherOverrides)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:MILE][u:YRD]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:MILE][u:ACRE]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with override units that are inconsistent with each other");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_DuplicateOverrides)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:MILE][u:YRD]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:MILE][u:MILE]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with override units that are duplicates ");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_InvalidOrderingOfOverrides)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:MILE][u:YRD]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:YRD][u:MILE]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with override units that in the wrong order. Major unit must be biggest ");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_CantOverrideUnitsIfTheyAlreadyAreDefined)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI[u:M][u:MILE]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with override units that Don't match those already defined in the parent format");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_CantOverrideMoreUnitsThanAreDefined)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI[u:FT][u:IN|banana]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI[u:FT][u:IN|banana][u:MILLIINCH|woops]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with more override units than are defined in the parent format (except in the case of zero already defined)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_CantOverrideMoreThanFourUnits)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:KM][u:M|banana][u:DM][u:CM]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:KM][u:M|banana][u:DM][u:CM][u:MM|woops]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with more than 4 override units");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_InvalidDecimalPrecisionValues)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:M]" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU(-1)[u:M]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a bad precision value ");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU(13)[u:M]" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a bad precision value ");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, Fail_InvalidFractionalPrecisionValues)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI" relativeError="10e-3" />
                                        </ECSchema>)xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI(-1)" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a bad precision value ");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI(3)" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a bad precision value ");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI(512)" relativeError="10e-3" />
                                        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with a bad precision value ");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FormatStringNoOverrides)
    {
    SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI" relativeError="10e-3" />
                                        </ECSchema>)xml");

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");

    auto format = koq->GetDefaultPresentationFormat();

    ASSERT_NE(nullptr, format);
    EXPECT_STRCASEEQ("AmerFI", format->GetName().c_str());
    EXPECT_TRUE(format->IsIdentical(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FormatStringOverrideFormatWithUnits)
    {
    SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:AmerFI[u:FT|fish][u:IN|bone]" relativeError="10e-3" />
                                        </ECSchema>)xml");

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");

    auto format = koq->GetDefaultPresentationFormat();

    ASSERT_NE(nullptr, format);
    EXPECT_STRCASEEQ("AmerFI[u:FT|fish][u:IN|bone]", format->GetName().c_str());
    EXPECT_STRCASEEQ("FT", format->GetCompositeMajorUnit()->GetName().c_str());
    EXPECT_STRCASEEQ("IN", format->GetCompositeMiddleUnit()->GetName().c_str());
    auto comp = format->GetCompositeSpec();
    EXPECT_STRCASEEQ("fish", comp->GetMajorLabel().c_str());
    EXPECT_STRCASEEQ("bone", comp->GetMiddleLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FormatStringOverrideUnitsAndLabels)
    {
    SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                                            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:MILE|m][u:YRD|y][u:IN|i][u:MILLIINCH|mi]" relativeError="10e-3" />
                                        </ECSchema>)xml");

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");

    auto format = koq->GetDefaultPresentationFormat();

    ASSERT_NE(nullptr, format);
    EXPECT_STRCASEEQ("DefaultRealU[u:MILE|m][u:YRD|y][u:IN|i][u:MILLIINCH|mi]", format->GetName().c_str());
    EXPECT_STRCASEEQ("MILE", format->GetCompositeMajorUnit()->GetName().c_str());
    EXPECT_STRCASEEQ("YRD", format->GetCompositeMiddleUnit()->GetName().c_str());
    EXPECT_STRCASEEQ("IN", format->GetCompositeMinorUnit()->GetName().c_str());
    EXPECT_STRCASEEQ("MILLIINCH", format->GetCompositeSubUnit()->GetName().c_str());

    auto comp = format->GetCompositeSpec();
    EXPECT_STRCASEEQ("m", comp->GetMajorLabel().c_str());
    EXPECT_STRCASEEQ("y", comp->GetMiddleLabel().c_str());
    EXPECT_STRCASEEQ("i", comp->GetMinorLabel().c_str());
    EXPECT_STRCASEEQ("mi", comp->GetSubLabel().c_str());
    }

//=======================================================================================
//! KindOfQuantitySerializationTest
//=======================================================================================

Utf8CP KindOfQuantitySerializationTest::s_testSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
        <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
        <KindOfQuantity typeName="TestKoQ" relativeError="10e-3" persistenceUnit="u:M" presentationUnits="f:DefaultRealU[u:CM]" />
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
    EXPECT_STRCASEEQ("DefaultRealU[u:CM]", pres->GetName().c_str());
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
    EXPECT_STRCASEEQ("DefaultRealU[u:CM]", pres->GetName().c_str());
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
    EXPECT_STRCASEEQ("DefaultRealU[u:CM]", pres->GetName().c_str());
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
    EXPECT_EQ(ECObjectsStatus::Success, koq->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));

    Utf8String xmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, origSchema->WriteToXmlString(xmlString, ECVersion::Latest));

    ECSchemaPtr readSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(readSchema, xmlString.c_str(), *context));
    auto koq2 = readSchema->GetKindOfQuantityCP("ExampleKoQ");
    auto persist = koq2->GetPersistenceUnit();
    auto pres = koq2->GetDefaultPresentationFormat();
    EXPECT_STRCASEEQ("M", persist->GetName().c_str());
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", pres->GetName().c_str());
    }
    }

//=======================================================================================
//! KindOfQuantityRoundTripTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityRoundTripTest, KindOfQuantityCreatedFromAPI)
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
    kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    kindOfQuantity->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultReal"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));

    // TODO: Remove the property aspect of this test.
    ECEntityClassP entityClass;
    ASSERT_TRUE(schema->CreateEntityClass(entityClass, "EntityClass") == ECObjectsStatus::Success);
    PrimitiveArrayECPropertyP property;
    auto status = entityClass->CreatePrimitiveArrayProperty(property, "QuantifiedProperty", PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(property != nullptr);
    property->SetKindOfQuantity(kindOfQuantity);

    ECSchemaPtr deserializedSchema;
    RoundTripSchema(deserializedSchema, schema.get(), ECVersion::Latest);

    EXPECT_EQ(1, deserializedSchema->GetKindOfQuantityCount());
    KindOfQuantityCP deserializedKindOfQuantity;
    deserializedKindOfQuantity = deserializedSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(deserializedKindOfQuantity != nullptr);
    EXPECT_STREQ("DL", deserializedKindOfQuantity->GetDisplayLabel().c_str());
    EXPECT_STREQ("DESC", deserializedKindOfQuantity->GetDescription().c_str());
    EXPECT_STREQ("CM", deserializedKindOfQuantity->GetPersistenceUnit()->GetName().c_str());
    EXPECT_EQ(10e-3, deserializedKindOfQuantity->GetRelativeError());

    EXPECT_STREQ("DefaultRealU[u:M]", deserializedKindOfQuantity->GetDefaultPresentationFormat()->GetName().c_str());
    auto& resultAltUnits = deserializedKindOfQuantity->GetPresentationFormats();
    EXPECT_EQ(3, resultAltUnits.size()); // Default presentation unit is included in list of presentation units
    EXPECT_STREQ("DefaultReal[u:M]", resultAltUnits[1].GetName().c_str());
    EXPECT_STREQ("AmerFI", resultAltUnits[2].GetName().c_str());

    // TODO: Remove the property aspect of this test.
    ECClassCP deserializedClass = deserializedSchema->GetClassCP("EntityClass");
    ECPropertyP deserializedProperty = deserializedClass->GetPropertyP("QuantifiedProperty");
    KindOfQuantityCP propertyKoq = deserializedProperty->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
    ASSERT_TRUE(nullptr != propertyKoq);
    EXPECT_EQ(propertyKoq, deserializedKindOfQuantity);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, Fail_ec31_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"
                        presentationUnits="f:DefaultRealU(9)[u:IN|banana];f:DefaultRealU[u:M]" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchema(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::InvalidECSchemaXml, SchemaWriteStatus::FailedToSaveXml, "Should fail to round trip a 3.1 schema with KoQ using EC3.2 unit defined in schema as persistence unit");
    ASSERT_FALSE(schema.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec31_roundTripShouldDropUnknownPresentationFormats)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationUnits="f:DefaultRealU(9)[TestUnit|banana];f:AmerFI" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchema(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should succeed to round trip a 3.1 schema with KoQ using EC3.2 unit defined in schema as presentation unit");
    ASSERT_TRUE(schema.IsValid());
    ASSERT_EQ(1, schema->GetKindOfQuantityCP("MyKindOfQuantity")->GetPresentationFormats().size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, Fail_ec30_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"
                        presentationUnits="f:DefaultRealU(9)[u:IN|banana];f:DefaultRealU[u:M]" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchema(schema, schemaItem, ECVersion::V3_0, SchemaReadStatus::InvalidECSchemaXml, SchemaWriteStatus::FailedToSaveXml, "Should fail to round trip a 3.0 schema with KoQ using EC3.2 unit defined in schema as persistence unit");
    ASSERT_FALSE(schema.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec30_roundTripShouldDropUnknownPresentationFormats)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationUnits="f:DefaultRealU(9)[TestUnit|banana];f:AmerFI" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchema(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should succeed to round trip a 3.0 schema with KoQ using EC3.2 unit defined in schema as presentation unit");
    ASSERT_TRUE(schema.IsValid());
    ASSERT_EQ(1, schema->GetKindOfQuantityCP("MyKindOfQuantity")->GetPresentationFormats().size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityRoundTripTest, ec31_roundTrip)
    {
    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationUnits="f:AmerFI;f:DefaultRealU[u:M]" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchema(schema, schemaItem, ECVersion::V3_1, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should be able to round trip a schema from 3.2 -> 3.1 -> 3.2");
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("AmerFI", koq->GetPresentationFormats()[0].GetName().c_str());
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", koq->GetPresentationFormats()[1].GetName().c_str());
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
        <ECSchema schemaName="Schema2" alias="s2" version="1.00.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:FT" relativeError=".5"
                        presentationUnits="f:AmerFI;f:DefaultRealU[u:M]" />
            <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="MyKindOfQuantity" />
                <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="MyKindOfQuantity"/>
                <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
        </ECSchema>)xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    RoundTripSchema(schema, schemaItem, ECVersion::V3_0, SchemaReadStatus::Success, SchemaWriteStatus::Success, "Should be able to round trip a schema from 3.2 -> 3.0 -> 3.2");
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("AmerFI", koq->GetPresentationFormats()[0].GetName().c_str());
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", koq->GetPresentationFormats()[1].GetName().c_str());
    EXPECT_DOUBLE_EQ(0.5, koq->GetRelativeError());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDescription().c_str());
    EXPECT_STRCASEEQ("My KindOfQuantity", koq->GetInvariantDisplayLabel().c_str());
    }

//=======================================================================================
//! KindOfQuantityUpgradeTest
//=======================================================================================

void KindOfQuantityUpgradeTest::VerifySchemaReferencesUnitsSchema(ECSchemaPtr schema)
    {
    EXPECT_TRUE(SchemaReferencesUnitsSchema(schema));
    }

void KindOfQuantityUpgradeTest::VerifySchemaReferencesFormatsSchema(ECSchemaPtr schema)
    {
    EXPECT_TRUE(SchemaReferencesFormatsSchema(schema));
    }

bool KindOfQuantityUpgradeTest::SchemaReferencesUnitsSchema(ECSchemaPtr schema)
    {
    return ECSchema::IsSchemaReferenced(*schema, *GetUnitsSchema());
    }

bool KindOfQuantityUpgradeTest::SchemaReferencesFormatsSchema(ECSchemaPtr schema)
    {
    return ECSchema::IsSchemaReferenced(*schema, *GetFormatsSchema());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_UnknownFormat)
    {
            {
            // Unknown Persistence Format. The format should be dropped but the persistenceUnit should succeed.
            // old persistenceFUS: MM(SillyFormat)
            // old presentationFUS': -
            // new persistenceUnit: u:MM
            // new presentationFormats: -
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            DeserializeSchema(schema, *context, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="K1" persistenceUnit="M(SILLYFORMAT)" relativeError="10e-3" />
        </ECSchema>)xml"), SchemaReadStatus::Success, "Schema should succeed even though the persistence FUS has an invalid format.");

            EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
            VerifySchemaReferencesUnitsSchema(schema);

            auto koq = schema->GetKindOfQuantityCP("K1");
            ASSERT_NE(nullptr, koq);
            EXPECT_NE(nullptr, koq->GetPersistenceUnit());
            EXPECT_STREQ("Units:M", koq->GetPersistenceUnit()->GetFullName().c_str());
            EXPECT_EQ(0, koq->GetPresentationFormats().size());
            }
            {
            // Unknown Presentation Format. It should be dropped.
            // old persistenceFUS: MM
            // old presentationFUS': MM(KindOfFormat)
            // new persistenceUnit: u:MM
            // new presentationFormats: -
            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            DeserializeSchema(schema, *context, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="K1" persistenceUnit="MM" presentationUnits="MM(UnknownFormat)" relativeError="5" />
        </ECSchema>)xml"), SchemaReadStatus::Success, "Schema should succeed even though a format mapping is not available");

            EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
            VerifySchemaReferencesUnitsSchema(schema);

            auto koq = schema->GetKindOfQuantityCP("K1");
            ASSERT_NE(nullptr, koq);
            EXPECT_NE(nullptr, koq->GetPersistenceUnit());
            EXPECT_STREQ("MM", koq->GetPersistenceUnit()->GetName().c_str());
            ASSERT_NE(nullptr, koq->GetDefaultPresentationFormat());
            EXPECT_EQ(0, koq->GetPresentationFormats().size());
            }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_NoPersistenceFormatIfPresentationFUSesDefined)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    DeserializeSchema(schema, *schemaContext, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" persistenceUnit="CM" presentationUnits="FT;IN" relativeError=".5" />
        </ECSchema>)xml"));

    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    VerifySchemaReferencesUnitsSchema(schema);
    VerifySchemaReferencesFormatsSchema(schema);

    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("DefaultReal[u:FT]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("DefaultReal[u:IN]", koq->GetPresentationFormats()[1].GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                   04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_OnlyPersistenceUnit)
    {
            { // Fail to upgrade if there is an invalid unit.
            // old persistenceFUS: badUnit
            // old presentationFUS': -
            // new persistenceUnit: -
            // new presentationFormats: -
            ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown persistence unit");
            }
            {
            // old persistenceFUS: MM
            // old presentationFUS': -
            // new persistenceUnit: u:MM
            // new presentationFormats: -
            ECSchemaPtr schema;
            ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
            DeserializeSchema(schema, *schemaContext, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" persistenceUnit="MM" relativeError=".5" />
        </ECSchema>)xml"));

            EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
            VerifySchemaReferencesUnitsSchema(schema);

            auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
            ASSERT_NE(nullptr, koq);
            EXPECT_STREQ("Units:MM", koq->GetPersistenceUnit()->GetFullName().c_str());
            EXPECT_EQ(0, koq->GetPresentationFormats().size());
            }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                   04/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_InvalidPresentationUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="SILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Schema should fail to deserialize with an unknown presentation Unit");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    05/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_IncompatibleFUSPresentationUnitAndFormat)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    DeserializeSchema(schema, *schemaContext, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="LENGTH_SHORT" persistenceUnit="M(DefaultReal)" presentationUnits="IN(fi8)"
                    relativeError="0.01" />
        </ECSchema>)xml"));

    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    VerifySchemaReferencesUnitsSchema(schema);
    VerifySchemaReferencesFormatsSchema(schema);

    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("LENGTH_SHORT");
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    NamedFormatCP format = koq->GetDefaultPresentationFormat();
    EXPECT_TRUE(format->HasCompositeMajorUnit());
    EXPECT_STREQ("FT", format->GetCompositeMajorUnit()->GetName().c_str());
    EXPECT_TRUE(format->HasCompositeMiddleUnit());
    EXPECT_STREQ("IN", format->GetCompositeMiddleUnit()->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                06/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityUpgradeTest, ec31_formatWithMutipleUnitsButOnlyOneInputUnitShouldUpgradeProperly)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    DeserializeSchema(schema, *schemaContext, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="Test"  persistenceUnit="FT(AmerFI8)" presentationUnits="IN(AmerFI8)" relativeError="0.6"/>
            <KindOfQuantity typeName="Test1"  persistenceUnit="FT(AmerFI8)" relativeError="0.6"/>
        </ECSchema>)xml"));

    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));
    VerifySchemaReferencesUnitsSchema(schema);
    VerifySchemaReferencesFormatsSchema(schema);

    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("Test");
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    NamedFormatCP format = koq->GetDefaultPresentationFormat();
    EXPECT_TRUE(format->HasCompositeMajorUnit());
    EXPECT_STREQ("FT", format->GetCompositeMajorUnit()->GetName().c_str());
    EXPECT_TRUE(format->HasCompositeMiddleUnit());
    EXPECT_STREQ("IN", format->GetCompositeMiddleUnit()->GetName().c_str());

    koq = schema->GetKindOfQuantityCP("Test1");
    EXPECT_EQ(1, koq->GetPresentationFormats().size());
    format = koq->GetDefaultPresentationFormat();
    EXPECT_TRUE(format->HasCompositeMajorUnit());
    EXPECT_STREQ("FT", format->GetCompositeMajorUnit()->GetName().c_str());
    EXPECT_TRUE(format->HasCompositeMiddleUnit());
    EXPECT_STREQ("IN", format->GetCompositeMiddleUnit()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Kyle.Abramowitz    06/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityUpgradeTest, ReferencesToUnitsAndFormatsAddedCorrectly)
    {
            {
            SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
                                            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
                                            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="TestUnit" relativeError="10e-3" />
                                        </ECSchema>)xml");

            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
            ASSERT_FALSE(SchemaReferencesFormatsSchema(schema));
            ASSERT_FALSE(SchemaReferencesUnitsSchema(schema));
            }

            {
            SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
                                            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
                                            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
                                            <Format typeName="TestFormat" type="decimal" precision="4">
                                                <Composite>
                                                    <Unit>TestUnit</Unit>
                                                </Composite>
                                            </Format>
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="TestUnit" presentationUnits="TestFormat[TestUnit|test]" relativeError="10e-3" />
                                        </ECSchema>)xml");

            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
            ASSERT_FALSE(SchemaReferencesFormatsSchema(schema));
            ASSERT_FALSE(SchemaReferencesUnitsSchema(schema));
            }

            {
            SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="M" relativeError="10e-3" />
                                        </ECSchema>)xml");

            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
            VerifySchemaReferencesUnitsSchema(schema);
            ASSERT_FALSE(SchemaReferencesFormatsSchema(schema));
            }

            {
            SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="M(real)" relativeError="10e-3" />
                                        </ECSchema>)xml");

            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
            VerifySchemaReferencesUnitsSchema(schema);
            VerifySchemaReferencesFormatsSchema(schema);
            }

            {
            SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="MM" relativeError="10e-3" />
                                        </ECSchema>)xml");

            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
            VerifySchemaReferencesUnitsSchema(schema);
            VerifySchemaReferencesFormatsSchema(schema);
            }

            {
            SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                                        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                                                displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="MM(real)" relativeError="10e-3" />
                                        </ECSchema>)xml");

            ECSchemaPtr schema;
            ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
            ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context));
            VerifySchemaReferencesUnitsSchema(schema);
            VerifySchemaReferencesFormatsSchema(schema);
            }
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
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <KindOfQuantity typeName="KoQWithPers" description="Kind of a Description here"
                displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an unknown perisistence unit in a newer (3.3) version");
    // Presentation FUS
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <KindOfQuantity typeName="KoQWithPres" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultReal[ANOTHERSILLYMETER];" relativeError="10e-3" />
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
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <KindOfQuantity typeName="KoQWithPres" description="Kind of a Description here"
                displayLabel="best quantity of all time" persistenceUnit="u:M" presentationUnits="f:DefaultReal;SILLYFORMAT" relativeError="10e-3" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid presentation format in EC 3.3");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCompatibilityTest, ec33_ValidKindOfQuantityInReferencedSchema)
    {
    SchemaItem refSchemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5"
                        presentationUnits="f:AmerFI;f:DefaultRealU[u:M]" />
        </ECSchema>)xml");

    SchemaItem schemaItem = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
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
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("AmerFI", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", koq->GetPresentationFormats()[1].GetName().c_str());
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
        <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5"
                        presentationUnits="f:AmerFI;f:DefaultRealU[u:M]" />
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
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("AmerFI", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("DefaultRealU[u:M]", koq->GetPresentationFormats()[1].GetName().c_str());
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
        <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                        displayLabel="My KindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"
                        presentationUnits="f:DefaultRealU[TestUnit];f:AmerFI;" />
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
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STRCASEEQ("DefaultRealU[TestUnit]", koq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_STRCASEEQ("AmerFI", koq->GetPresentationFormats()[1].GetName().c_str());
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
