/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../ECObjectsTestPCH.h"
#include "../../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct KindOfQuantityCopyTest : ECTestFixture {};

void copyKOQ_TargetRefSchemasAreUsedToLookUpReferencedUnitsAndFormats(ECSchemaCP testSchemaSource, ECSchemaP testSchemaTarget, ECSchemaP unitsSchemaTarget, bool copyReferences)
    {
    testSchemaTarget->AddReferencedSchema(*unitsSchemaTarget);
    KindOfQuantityCP koqSource = testSchemaSource->GetKindOfQuantityCP("myLength");
    KindOfQuantityP koqTarget;
    ASSERT_EQ(ECObjectsStatus::Success, testSchemaTarget->CopyKindOfQuantity(koqTarget, *koqSource, false));
    
    auto const unitTarget = koqTarget->GetPersistenceUnit();
    ASSERT_NE(nullptr, unitTarget);
    ASSERT_STREQ(unitsSchemaTarget->GetFullSchemaName().c_str(), unitTarget->GetSchema().GetFullSchemaName().c_str());
    auto const presentationTarget = koqTarget->GetDefaultPresentationFormat();
    ASSERT_NE(nullptr, presentationTarget);
    ASSERT_STREQ(unitsSchemaTarget->GetFullSchemaName().c_str(), presentationTarget->GetParentFormat()->GetSchema().GetFullSchemaName().c_str());
    
    auto const presentationSource = koqSource->GetDefaultPresentationFormat();
    ASSERT_NE(presentationSource, presentationTarget);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCopyTest, TargetRefSchemasAreUsedToLookUpReferencedUnitsAndFormats)
    {
    Utf8CP unitsSchema = R"**(
        <ECSchema schemaName="TestUnits" alias="tu" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI" />
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Format typeName="DefaultRealU" displayLabel="realu" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )**";
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestUnits" version="01.00.00" alias="tu"/>
            <KindOfQuantity typeName="myLength" persistenceUnit="tu:M" relativeError="0.001" presentationUnits="tu:DefaultRealU(4)[tu:M]"/>
        </ECSchema>
        )**";

    ECSchemaPtr unitsSource;
    ECSchemaPtr testSchemaSource;
    auto const schemaContextSource = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(unitsSource, unitsSchema, *schemaContextSource));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(testSchemaSource, schemaString, *schemaContextSource));

    Utf8CP unitsXmlTarget = R"**(
        <ECSchema schemaName="TestUnits" alias="tu" version="01.00.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI" />
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Format typeName="DefaultRealU" displayLabel="realu" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )**";

    ECSchemaPtr unitsSchemaTarget;
    auto schemaContextTarget = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(unitsSchemaTarget, unitsXmlTarget, *schemaContextTarget));


    // Copy References set to false
    ECSchemaPtr testSchemaTarget;
    ECSchema::CreateSchema(testSchemaTarget, "testSchema", "ts", 1, 0, 0);
    copyKOQ_TargetRefSchemasAreUsedToLookUpReferencedUnitsAndFormats(testSchemaSource.get(), testSchemaTarget.get(), unitsSchemaTarget.get(), false);

    // Copy References set to true
    ECSchema::CreateSchema(testSchemaTarget, "testSchema", "ts", 1, 0, 0);
    copyKOQ_TargetRefSchemasAreUsedToLookUpReferencedUnitsAndFormats(testSchemaSource.get(), testSchemaTarget.get(), unitsSchemaTarget.get(), true);

    Utf8CP units2XmlTarget = R"**(
        <ECSchema schemaName="TestUnits" alias="tu" version="02.00.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI" />
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Format typeName="DefaultRealU" displayLabel="realu" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )**";
    ECSchemaPtr units2SchemaTarget;
    schemaContextTarget = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(units2SchemaTarget, units2XmlTarget, *schemaContextTarget));

    // The referenced schema is a different major version
    ECSchema::CreateSchema(testSchemaTarget, "testSchema", "ts", 1, 0, 0);
    copyKOQ_TargetRefSchemasAreUsedToLookUpReferencedUnitsAndFormats(testSchemaSource.get(), testSchemaTarget.get(), units2SchemaTarget.get(), false);
    }

void copyKOQ_LocallyDefinedUnitAndFormatRefsAreCopiedProperly (ECSchemaCP sourceSchema, ECSchemaP targetSchema, Utf8CP testCase)
    {
    KindOfQuantityCP koqSource = sourceSchema->GetKindOfQuantityCP("myLength");
    KindOfQuantityP koqTarget;
    ASSERT_EQ(ECObjectsStatus::NotFound, targetSchema->CopyKindOfQuantity(koqTarget, *koqSource, false)) << testCase;
    ASSERT_EQ(nullptr, targetSchema->GetKindOfQuantityCP("myLength")) << testCase; // Ensure failed copy removed partial KOQ

    ASSERT_EQ(ECObjectsStatus::Success, targetSchema->CopyKindOfQuantity(koqTarget, *koqSource, true)) << testCase;

    ASSERT_EQ(0, targetSchema->GetReferencedSchemas().size()) << testCase;
    
    auto const unitTarget = koqTarget->GetPersistenceUnit();
    ASSERT_NE(nullptr, unitTarget) << testCase;
    ASSERT_NE(sourceSchema, &unitTarget->GetSchema()) << testCase;
    auto const presentationTarget = koqTarget->GetDefaultPresentationFormat();
    ASSERT_NE(nullptr, presentationTarget) << testCase;
    ASSERT_NE(sourceSchema, &presentationTarget->GetParentFormat()->GetSchema()) << testCase;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCopyTest, LocallyDefinedUnitAndFormatRefsAreCopiedProperly)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI" />
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Format typeName="DefaultRealU" displayLabel="realu" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
            <KindOfQuantity typeName="myLength" persistenceUnit="M" relativeError="0.001" presentationUnits="DefaultRealU(4)[M]"/>
        </ECSchema>
        )**";

    ECSchemaPtr sourceSchema;
    auto const schemaContextSource = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(sourceSchema, schemaString, *schemaContextSource));

    ECSchemaPtr targetSchema;
    ECSchema::CreateSchema(targetSchema, "testSchema", "ts", 1, 0, 0);
    copyKOQ_LocallyDefinedUnitAndFormatRefsAreCopiedProperly(sourceSchema.get(), targetSchema.get(), "name and version are identical");

    ECSchemaPtr targetSchema2;
    ECSchema::CreateSchema(targetSchema2, "testSchema", "ts", 2, 0, 5);
    copyKOQ_LocallyDefinedUnitAndFormatRefsAreCopiedProperly(sourceSchema.get(), targetSchema2.get(), "name is identical and version is different");

    // Copy when name and version is different
    // if copyReferences is false a reference is added
    ECSchemaPtr someOtherSchema;
    ECSchema::CreateSchema(someOtherSchema, "SomeOtherSchema", "sos", 42, 42, 42);
    KindOfQuantityCP koqSource = sourceSchema->GetKindOfQuantityCP("myLength");
    KindOfQuantityP koqTarget;
    ASSERT_EQ(ECObjectsStatus::Success, someOtherSchema->CopyKindOfQuantity(koqTarget, *koqSource, false));
    ASSERT_EQ(1, someOtherSchema->GetReferencedSchemas().size());
    
    auto unitTarget = koqTarget->GetPersistenceUnit();
    ASSERT_NE(nullptr, unitTarget);
    ASSERT_STREQ(sourceSchema->GetFullSchemaName().c_str(), unitTarget->GetSchema().GetFullSchemaName().c_str());
    auto presentationTarget = koqTarget->GetDefaultPresentationFormat();
    ASSERT_NE(nullptr, presentationTarget);
    ASSERT_STREQ(sourceSchema->GetFullSchemaName().c_str(), presentationTarget->GetParentFormat()->GetSchema().GetFullSchemaName().c_str());

    // if copyReferences is true the locally referenced things are copied
    ECSchemaPtr someOtherSchema2;
    ECSchema::CreateSchema(someOtherSchema2, "SomeOtherSchema", "sos", 42, 42, 42);
    ASSERT_EQ(ECObjectsStatus::Success, someOtherSchema2->CopyKindOfQuantity(koqTarget, *koqSource, true));

    ASSERT_EQ(0, someOtherSchema2->GetReferencedSchemas().size());
    
    unitTarget = koqTarget->GetPersistenceUnit();
    ASSERT_NE(nullptr, unitTarget);
    ASSERT_STREQ(someOtherSchema2->GetFullSchemaName().c_str(), unitTarget->GetSchema().GetFullSchemaName().c_str());
    presentationTarget = koqTarget->GetDefaultPresentationFormat();
    ASSERT_NE(nullptr, presentationTarget);
    ASSERT_STREQ(someOtherSchema2->GetFullSchemaName().c_str(), presentationTarget->GetParentFormat()->GetSchema().GetFullSchemaName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantity)
    {
    ECSchemaPtr sourceSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(sourceSchema, "TestSchema", "ts", 1, 0, 0));

    KindOfQuantityP koq;
    EC_ASSERT_SUCCESS(sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    sourceSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));
    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->SetRelativeError(10e-3);

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(sourceSchema, targetSchema);

    EXPECT_EQ(0, targetSchema->GetUnitCount());
    EXPECT_EQ(0, targetSchema->GetFormatCount());
    EXPECT_EQ(1, targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());

    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit()->GetName().c_str());
    
    const auto& formats = targetKoq->GetPresentationFormats();
    EXPECT_EQ(2, formats.size());

    EXPECT_STREQ("AmerFI", targetKoq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_EQ(ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"), targetKoq->GetDefaultPresentationFormat()->GetParentFormat());

    EXPECT_STREQ("DefaultRealU[u:M]", formats.at(1).GetName().c_str());
    EXPECT_EQ(ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), formats.at(1).GetParentFormat());
    EXPECT_NE(nullptr, formats.at(1).GetCompositeMajorUnit());
    EXPECT_EQ(ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), formats.at(1).GetCompositeMajorUnit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantity_NoPresentationFormats)
    {
    ECSchemaPtr sourceSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(sourceSchema, "TestSchema", "ts", 1, 0, 0));

    KindOfQuantityP koq;
    EC_ASSERT_SUCCESS(sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->SetRelativeError(10e-3);

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(sourceSchema, targetSchema);

    EXPECT_EQ(1, targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    CopyTestFixture::ValidateNameDescriptionAndDisplayLabel(*koq, *targetKoq);
    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_FALSE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantity_PersistenceUnitDefinedInSchema)
    {
    ECSchemaPtr sourceSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(sourceSchema, "TestSchema", "ts", 1, 0, 0));

    KindOfQuantityP koq;
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTELABEL", "SMOOT_SYSTEDESCRIPTION"));
    EC_ASSERT_SUCCESS(sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOLABEL", "SMOOT_PHENODESCRIPTION"));
    EC_ASSERT_SUCCESS(sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*sourceSchema->GetUnitCP("SMOOT"));
    koq->SetRelativeError(10e-3);

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(sourceSchema, targetSchema);

    EXPECT_EQ(1, targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_NE(targetKoq, koq);
    CopyTestFixture::ValidateNameDescriptionAndDisplayLabel(*koq, *targetKoq);
    EXPECT_FALSE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());

    ECUnitCP targetSmoot = targetSchema->GetUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetSmoot);
    EXPECT_STREQ(koq->GetPersistenceUnit()->GetName().c_str(), targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_NE(koq->GetPersistenceUnit(), targetSmoot);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantity_UnitAndFormaDefinedInSchemaHasSameNameAsReferencedUsedInKOQ)
    {
    ECSchemaPtr sourceSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(sourceSchema, "TestSchema", "ts", 1, 0, 0));
    const auto unitsSchema = ECTestFixture::GetUnitsSchema();
    const auto meter = unitsSchema->GetUnitCP("M");
    ASSERT_NE(nullptr, meter);
    const auto inch = unitsSchema->GetUnitCP("IN");
    ASSERT_NE(nullptr, inch);

    const auto formatSchema = ECTestFixture::GetFormatsSchema();
    const auto format = formatSchema->GetFormatCP("DefaultRealU");
    ASSERT_NE(nullptr, format);
    const auto format2 = formatSchema->GetFormatCP("AmerI");
    ASSERT_NE(nullptr, format2);

    sourceSchema->AddReferencedSchema(*unitsSchema);
    sourceSchema->AddReferencedSchema(*formatSchema);


    KindOfQuantityP koq;
    ECUnitP meterDupe;
    ECFormatP formatDupe;
    ECFormatP formatDupe2;
    EC_ASSERT_SUCCESS(sourceSchema->CreateUnit(meterDupe, "M", "u:M", *unitsSchema->GetPhenomenonP("LENGTH"), *unitsSchema->GetUnitSystemP("SI")));
    EC_ASSERT_SUCCESS(sourceSchema->CreateFormat(formatDupe, "DefaultRealU", "real"));
    const auto spec = Formatting::CompositeValueSpec(*inch);
    EC_ASSERT_SUCCESS(sourceSchema->CreateFormat(formatDupe2, "AmerI", "Inches", nullptr, nullptr, &spec));
    EC_ASSERT_SUCCESS(sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*meter);
    EC_ASSERT_SUCCESS(koq->AddPresentationFormat(*format2));
    EC_ASSERT_SUCCESS(koq->AddPresentationFormatSingleUnitOverride(*format, 12, meter));
    koq->SetRelativeError(10e-3);
    ASSERT_EQ(2, koq->GetPresentationFormats().size());

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(sourceSchema, targetSchema);

    EXPECT_EQ(1, targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_NE(targetKoq, koq);
    const auto persistenceUnitTarget = targetKoq->GetPersistenceUnit();
    ASSERT_STREQ(unitsSchema->GetFullSchemaName().c_str(), persistenceUnitTarget->GetSchema().GetFullSchemaName().c_str());
    const auto targetFormats = targetKoq->GetPresentationFormats();
    ASSERT_EQ(2, targetFormats.size());

    for (auto targetFormat : targetFormats)
        {
        ASSERT_STREQ(formatSchema->GetFullSchemaName().c_str(), targetFormat.GetParentFormat()->GetSchema().GetFullSchemaName().c_str());
        if (!targetFormat.GetParentFormat()->GetName().Equals("DefaultRealU"))
            targetFormat = *targetFormat.GetParentFormat();
        ASSERT_EQ(1, targetFormat.GetCompositeUnitCount()) << targetFormat.GetName().c_str();
        ASSERT_STREQ(unitsSchema->GetFullSchemaName().c_str(), (static_cast<ECUnitCP> (targetFormat.GetCompositeMajorUnit()))->GetSchema().GetFullSchemaName().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantity_PresentationFormatDefinedInSchema)
    {
    ECSchemaPtr sourceSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(sourceSchema, "TestSchema", "ts", 1, 0, 0));

    KindOfQuantityP koq;
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    ECFormatP format;
    EC_ASSERT_SUCCESS(sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTELABEL", "SMOOT_SYSTEDESCRIPTION"));
    EC_ASSERT_SUCCESS(sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOLABEL", "SMOOT_PHENODESCRIPTION"));
    EC_ASSERT_SUCCESS(sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(sourceSchema->CreateUnit(unit, "SMOOT_SQUARED", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(sourceSchema->CreateFormat(format, "SMOOT_FORMAT"));
    EC_ASSERT_SUCCESS(sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*sourceSchema->GetUnitCP("SMOOT"));
    koq->SetDefaultPresentationFormat(*format, nullptr, unit);
    koq->SetRelativeError(10e-3);

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(sourceSchema, targetSchema);

    EXPECT_EQ(1, targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = targetSchema->GetKindOfQuantityCP("TestKoQ");
    ECFormatCP targetFormat = targetSchema->GetFormatCP("SMOOT_FORMAT");
    ASSERT_TRUE(nullptr != targetKoq);
    ASSERT_TRUE(nullptr != targetFormat);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("SMOOT_FORMAT[SMOOT_SQUARED]", targetKoq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_TRUE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantityIncludingReferences)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="Unit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="M"/>
            <KindOfQuantity typeName="KindOfQuantity" persistenceUnit="Unit" relativeError=".5"/>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetUnitCP("Unit"));
    ASSERT_EQ(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    SchemaKey key("Units", 1, 0, 0);
    schemaCopyTo->AddReferencedSchema(*schemaContext->LocateSchema(key, SchemaMatchType::LatestReadCompatible));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyKindOfQuantity(koq, *schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(nullptr, schemaCopyTo->GetUnitCP("Unit"));
    EXPECT_NE(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyTo->GetUnitCP("Unit"));

    EXPECT_EQ(schemaCopyTo->GetUnitCP("Unit"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantityWithReferencedPresentationUnitSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="Unit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="M"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
            <KindOfQuantity typeName="KindOfQuantity" persistenceUnit="rs:Unit" relativeError=".5"/>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ASSERT_NE(nullptr, referenceSchema->GetUnitCP("Unit"));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"));
    ASSERT_EQ(referenceSchema->GetUnitCP("Unit"), schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    SchemaKey key("Units", 1, 0, 0);
    schemaCopyTo->AddReferencedSchema(*schemaContext->LocateSchema(key, SchemaMatchType::LatestReadCompatible));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyKindOfQuantity(koq, *schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetUnitCP("Unit"));

    EXPECT_EQ(referenceSchema->GetUnitCP("Unit"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(KindOfQuantityCopyTest, CopyKindOfQuantityWithReferencedSourceSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="Unit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="M"/>
            <KindOfQuantity typeName="KindOfQuantity" persistenceUnit="Unit" relativeError=".5"/>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetUnitCP("Unit"));
    ASSERT_EQ(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ns", 2, 0, 0);
    SchemaKey key("Units", 1, 0, 0);
    schemaCopyTo->AddReferencedSchema(*schemaContext->LocateSchema(key, SchemaMatchType::LatestReadCompatible));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyKindOfQuantity(koq, *schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetUnitCP("Unit"));

    EXPECT_EQ(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
