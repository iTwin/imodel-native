/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <ECObjects/SchemaComparer.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaCompareTest : ECTestFixture 
    {
    ECSchemaPtr m_firstSchema;
    ECSchemaPtr m_secondSchema;

    public:
        void CreateFirstSchema() {EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_firstSchema, "TestSchema", "ts", 1, 0, 0));}
        void CreateSecondSchema() {EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_secondSchema, "TestSchema", "ts", 1, 0, 0));}
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareSchemaWithUnitsSame)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    KindOfQuantityP koq;
    ECUnitP unit;
    ECFormatP format;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    auto num = Formatting::NumericFormatSpec();
    EC_ASSERT_SUCCESS(m_firstSchema->CreateFormat(format, "SMOOT4U", nullptr, nullptr, &num));
    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    koq->AddPresentationFormat(*format);

    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreateFormat(format, "SMOOT4U", nullptr, nullptr, &num));
    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    koq->AddPresentationFormat(*format);

    SchemaComparer comparer; 
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(0, changes.Changes().Count()); //Identical
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareSchemaWithDifferingUnitUnitSystemAndPhenomenon)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "APPLE", "BANANA"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "APPLE", "BANANA", "PEAR"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnit(unit, "SMOOT", "APPLE", *phenom, *system, "BANANA", "PEAR"));

    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());

    auto& firstChanges = changes.Changes()[0];

    ASSERT_EQ(3, firstChanges.MemberChangesCount());

    auto& unitChanges = firstChanges.Units();
    auto& phenomChanges = firstChanges.Phenomena();
    auto& systemChanges = firstChanges.UnitSystems();

    ASSERT_EQ(1, unitChanges.Count());
    ASSERT_EQ(1, phenomChanges.Count());
    ASSERT_EQ(1, systemChanges.Count());

    EXPECT_STREQ("SMOOT", unitChanges[0].Definition().GetOld().Value().c_str());
    EXPECT_STREQ("APPLE", unitChanges[0].Definition().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT", unitChanges[0].DisplayLabel().GetOld().Value().c_str());
    EXPECT_STREQ("BANANA", unitChanges[0].DisplayLabel().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT", unitChanges[0].Description().GetOld().Value().c_str());
    EXPECT_STREQ("PEAR", unitChanges[0].Description().GetNew().Value().c_str());

    EXPECT_STREQ("SMOOT", phenomChanges[0].Definition().GetOld().Value().c_str());
    EXPECT_STREQ("APPLE", phenomChanges[0].Definition().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT_PHENOM_LABEL", phenomChanges[0].DisplayLabel().GetOld().Value().c_str());
    EXPECT_STREQ("BANANA", phenomChanges[0].DisplayLabel().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT_PHENOM_DESCRIPTION", phenomChanges[0].Description().GetOld().Value().c_str());
    EXPECT_STREQ("PEAR", phenomChanges[0].Description().GetNew().Value().c_str());

    EXPECT_STREQ("SMOOT_SYSTEM_LABEL", systemChanges[0].DisplayLabel().GetOld().Value().c_str());
    EXPECT_STREQ("APPLE", systemChanges[0].DisplayLabel().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT_SYSTEM_DESCRIPTION", systemChanges[0].Description().GetOld().Value().c_str());
    EXPECT_STREQ("BANANA", systemChanges[0].Description().GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareShouldHandleUnitsWithDependenciesInOtherSchemas)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    ECSchemaPtr ref;
    ECSchema::CreateSchema(ref, "REF", "r", 1,0,0);
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));

    EC_ASSERT_SUCCESS(ref->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(ref->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    m_secondSchema->AddReferencedSchema(*ref);
    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));

    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());

    auto& firstChanges = changes.Changes()[0];

    ASSERT_EQ(4, firstChanges.MemberChangesCount());

    auto& unitChanges = firstChanges.Units();

    ASSERT_EQ(1, unitChanges.Count());
    
    EXPECT_STRCASEEQ("TestSchema:SMOOT_PHENOM", unitChanges[0].Phenomenon().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:SMOOT_PHENOM", unitChanges[0].Phenomenon().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("TestSchema:SMOOT_SYSTEM", unitChanges[0].UnitSystem().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:SMOOT_SYSTEM", unitChanges[0].UnitSystem().GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareKindOfQuantitiesWithUnitsInReferencedSchema)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    KindOfQuantityP koq;

    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    m_secondSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"));

    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(0, changes.Changes().Count()); //Identical
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareKindOfQuantitiesWithUnitsWithSameNameInDifferentReferencedSchemas)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    KindOfQuantityP koq;
    ECSchemaPtr ref;
    ECSchema::CreateSchema(ref, "REF", "r", 1,0,0);

    ECUnitP unit;
    ECFormatP format;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(ref->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(ref->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    m_secondSchema->AddReferencedSchema(*ref);
    EC_ASSERT_SUCCESS(ref->CreateUnit(unit, "M", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    auto num = Formatting::NumericFormatSpec();
    EC_ASSERT_SUCCESS(ref->CreateFormat(format, "AmerFI", nullptr, nullptr, &num));

    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    koq->AddPresentationFormatSingleUnitOverride(*format, nullptr, unit);

    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());

    auto& koqChanges = changes.Changes()[0].KindOfQuantities();

    ASSERT_EQ(1, changes.Changes().Count());

    auto& pers = koqChanges[0].PersistenceUnit();
    auto& pres = koqChanges[0].PresentationFormats();

    EXPECT_STRCASEEQ("Units:M", pers.GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:M", pers.GetNew().Value().c_str());

    ASSERT_EQ(1, pres.Count());

    EXPECT_FALSE(pres[0].GetOld().IsNull());
    EXPECT_STRCASEEQ("f:AmerFI", pres[0].GetOld().Value().c_str());

    EXPECT_FALSE(pres[0].GetNew().IsNull());
    EXPECT_STRCASEEQ("r:AmerFI[r:M]", pres[0].GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareKindOfQuantitiesWithUnitsInReferencedSchemaWithDifferences)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    KindOfQuantityP koq;

    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    EC_ASSERT_SUCCESS(m_firstSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_ASSERT_SUCCESS(m_firstSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));
    ASSERT_EQ(ECObjectsStatus::Success,koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    EC_ASSERT_SUCCESS(koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    ASSERT_EQ(1, koq->GetPresentationFormats().size());

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    EC_ASSERT_SUCCESS(m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_ASSERT_SUCCESS(m_secondSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));
    ASSERT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EC_ASSERT_SUCCESS(koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultReal"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    ASSERT_EQ(1, koq->GetPresentationFormats().size());

    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    ASSERT_EQ(1, changes.Changes()[0].MemberChangesCount());
    auto& koqChanges = changes.Changes()[0].KindOfQuantities();

    ASSERT_EQ(1, koqChanges.Count());

    EXPECT_STRCASEEQ("Units:CM", koqChanges[0].PersistenceUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Units:M", koqChanges[0].PersistenceUnit().GetNew().Value().c_str());

    auto& pres = koqChanges[0].PresentationFormats();
    ASSERT_EQ(1, pres.Count());

    ASSERT_FALSE(pres[0].GetOld().IsNull());
    EXPECT_STRCASEEQ("f:DefaultRealU[u:M]", pres[0].GetOld().Value().c_str());

    ASSERT_FALSE(pres[0].GetNew().IsNull());
    EXPECT_STRCASEEQ("f:DefaultReal[u:CM]", pres[0].GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsIdentical)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Bannana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");
    comp.SetSpacer("-");

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');
    num.SetRoundingFactor(0.1);

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(0, changes.Changes().Count()); //Identical
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      05/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsDeleted)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Bannana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");
    comp.SetSpacer("-");

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');
    num.SetRoundingFactor(0.1);

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());

    auto formatChanges = changes.Changes()[0].Formats();
    ASSERT_EQ(1, formatChanges.Count());

    auto formatChange = formatChanges[0];

    ASSERT_EQ(5, formatChange.MemberChangesCount());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsDeletedCompositeSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Banana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");
    comp.SetSpacer("Spacer1");
    auto num = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& formatChanges = changes.Changes()[0].Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsCompareToEmptyCompositeSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Banana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");
    comp.SetSpacer("Spacer1");
    auto comp2 = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    auto num = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp2);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& formatChanges = changes.Changes()[0].Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());

    EXPECT_STRCASEEQ("UNITS:DM", formatChange.CompositeSpec().MiddleUnit().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MiddleUnit().GetNew().IsNull());
    EXPECT_STRCASEEQ("UNITS:CM", formatChange.CompositeSpec().MinorUnit().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MinorUnit().GetNew().IsNull());
    EXPECT_STRCASEEQ("UNITS:MM", formatChange.CompositeSpec().SubUnit().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().SubUnit().GetNew().IsNull());

    EXPECT_STRCASEEQ("Spacer1", formatChange.CompositeSpec().Spacer().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().Spacer().GetNew().IsNull());

    EXPECT_STRCASEEQ("Apple", formatChange.CompositeSpec().MajorLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MajorLabel().GetNew().IsNull());
    EXPECT_STRCASEEQ("Banana", formatChange.CompositeSpec().MiddleLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MiddleLabel().GetNew().IsNull());
    EXPECT_STRCASEEQ("Carrot", formatChange.CompositeSpec().MinorLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MinorLabel().GetNew().IsNull());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.CompositeSpec().SubLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().SubLabel().GetNew().IsNull());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsNewCompositeSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Banana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");
    comp.SetSpacer("Spacer1");
    auto comp2 = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    auto num = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp2);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& formatChanges = changes.Changes()[0].Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());

    EXPECT_STRCASEEQ("UNITS:DM", formatChange.CompositeSpec().MiddleUnit().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MiddleUnit().GetOld().IsNull());
    EXPECT_STRCASEEQ("UNITS:CM", formatChange.CompositeSpec().MinorUnit().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MinorUnit().GetOld().IsNull());
    EXPECT_STRCASEEQ("UNITS:MM", formatChange.CompositeSpec().SubUnit().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().SubUnit().GetOld().IsNull());

    EXPECT_STRCASEEQ("Spacer1", formatChange.CompositeSpec().Spacer().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().Spacer().GetOld().IsNull());

    EXPECT_STRCASEEQ("Apple", formatChange.CompositeSpec().MajorLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MajorLabel().GetOld().IsNull());
    EXPECT_STRCASEEQ("Banana", formatChange.CompositeSpec().MiddleLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MiddleLabel().GetOld().IsNull());
    EXPECT_STRCASEEQ("Carrot", formatChange.CompositeSpec().MinorLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().MinorLabel().GetOld().IsNull());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.CompositeSpec().SubLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.CompositeSpec().SubLabel().GetOld().IsNull());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsModifiedCompositeSpecs)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Banana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");
    comp.SetSpacer("Spacer1");

    auto comp2 = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("MILE"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("YRD"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("FT"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("IN"));
    comp2.SetMajorLabel("Dragonfruit");
    comp2.SetMiddleLabel("Eggplant");
    comp2.SetMinorLabel("Fig");
    comp2.SetSubLabel("Grapefruit");
    comp2.SetSpacer("Spacer2");

    auto num = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp2);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& formatChanges = changes.Changes()[0].Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());

    EXPECT_STRCASEEQ("UNITS:M", formatChange.CompositeSpec().MajorUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:MILE", formatChange.CompositeSpec().MajorUnit().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:DM", formatChange.CompositeSpec().MiddleUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:YRD", formatChange.CompositeSpec().MiddleUnit().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:CM", formatChange.CompositeSpec().MinorUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:FT", formatChange.CompositeSpec().MinorUnit().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:MM", formatChange.CompositeSpec().SubUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:IN", formatChange.CompositeSpec().SubUnit().GetNew().Value().c_str());

    EXPECT_STRCASEEQ("Spacer1", formatChange.CompositeSpec().Spacer().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Spacer2", formatChange.CompositeSpec().Spacer().GetNew().Value().c_str());

    EXPECT_STRCASEEQ("Apple", formatChange.CompositeSpec().MajorLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.CompositeSpec().MajorLabel().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Banana", formatChange.CompositeSpec().MiddleLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Eggplant", formatChange.CompositeSpec().MiddleLabel().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Carrot", formatChange.CompositeSpec().MinorLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Fig", formatChange.CompositeSpec().MinorLabel().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.CompositeSpec().SubLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Grapefruit", formatChange.CompositeSpec().SubLabel().GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsDeletedNumericSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana");
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& change = changes.Changes()[0].Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change[0];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsCompareToEmptyNumericSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    auto num2 = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num2);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& change = changes.Changes()[0].Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());

    EXPECT_STRCASEEQ("applyRounding", formatChange.NumericSpec().FormatTraits().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().FormatTraits().GetNew().IsNull());
    EXPECT_STRCASEEQ("Fractional", formatChange.NumericSpec().PresentationType().GetOld().Value().c_str());
    EXPECT_EQ(128, formatChange.NumericSpec().FractionalPrecision().GetOld().Value());
    EXPECT_EQ(4, formatChange.NumericSpec().MinWidth().GetOld().Value());
    EXPECT_TRUE(formatChange.NumericSpec().MinWidth().GetNew().IsNull());
    EXPECT_STRCASEEQ("", formatChange.NumericSpec().UomSeparator().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().UomSeparator().GetNew().IsNull());
    EXPECT_STRCASEEQ("NegativeParentheses", formatChange.NumericSpec().ShowSignOption().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().ShowSignOption().GetNew().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.NumericSpec().DecimalSeparator().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().DecimalSeparator().GetNew().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.NumericSpec().ThousandsSeparator().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().ThousandsSeparator().GetNew().IsNull());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsNewNumericSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    auto num2 = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num2);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& change = changes.Changes()[0].Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());

    EXPECT_STRCASEEQ("applyRounding", formatChange.NumericSpec().FormatTraits().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().FormatTraits().GetOld().IsNull());
    EXPECT_STRCASEEQ("Fractional", formatChange.NumericSpec().PresentationType().GetNew().Value().c_str());
    EXPECT_EQ(128, formatChange.NumericSpec().FractionalPrecision().GetNew().Value());
    EXPECT_EQ(4, formatChange.NumericSpec().MinWidth().GetNew().Value());
    EXPECT_TRUE(formatChange.NumericSpec().MinWidth().GetOld().IsNull());
    EXPECT_STRCASEEQ("", formatChange.NumericSpec().UomSeparator().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().UomSeparator().GetOld().IsNull());
    EXPECT_STRCASEEQ("NegativeParentheses", formatChange.NumericSpec().ShowSignOption().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().ShowSignOption().GetOld().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.NumericSpec().DecimalSeparator().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().DecimalSeparator().GetOld().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.NumericSpec().ThousandsSeparator().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.NumericSpec().ThousandsSeparator().GetOld().IsNull());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      04/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsModifiedNumericSpec)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    auto num2 = Formatting::NumericFormatSpec();
    num2.SetExponentOnlyNegative(true);
    num2.SetPresentationType(Formatting::PresentationType::Decimal);
    num2.SetPrecision(Formatting::DecimalPrecision::Precision10);
    num2.SetMinWidth(8);
    num2.SetUomSeparator("smoot");
    num2.SetSignOption(Formatting::SignOption::OnlyNegative);
    num2.SetDecimalSeparator('.');
    num2.SetThousandSeparator('.');

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num2);
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Changes().Count());
    auto& change = changes.Changes()[0].Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change[0];

    ASSERT_EQ(1, formatChange.MemberChangesCount());

    EXPECT_STRCASEEQ("applyRounding", formatChange.NumericSpec().FormatTraits().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("exponentOnlyNegative", formatChange.NumericSpec().FormatTraits().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Fractional", formatChange.NumericSpec().PresentationType().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Decimal", formatChange.NumericSpec().PresentationType().GetNew().Value().c_str());
    EXPECT_EQ(128, formatChange.NumericSpec().FractionalPrecision().GetOld().Value());
    EXPECT_EQ(64, formatChange.NumericSpec().FractionalPrecision().GetNew().Value());
    EXPECT_EQ(6, formatChange.NumericSpec().DecimalPrecision().GetOld().Value());
    EXPECT_EQ(10, formatChange.NumericSpec().DecimalPrecision().GetNew().Value());
    EXPECT_EQ(4, formatChange.NumericSpec().MinWidth().GetOld().Value());
    EXPECT_EQ(8, formatChange.NumericSpec().MinWidth().GetNew().Value());
    EXPECT_STRCASEEQ("", formatChange.NumericSpec().UomSeparator().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("smoot", formatChange.NumericSpec().UomSeparator().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("NegativeParentheses", formatChange.NumericSpec().ShowSignOption().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("OnlyNegative", formatChange.NumericSpec().ShowSignOption().GetNew().Value().c_str());
    EXPECT_STRCASEEQ(",", formatChange.NumericSpec().DecimalSeparator().GetOld().Value().c_str());
    EXPECT_STRCASEEQ(".", formatChange.NumericSpec().DecimalSeparator().GetNew().Value().c_str());
    EXPECT_STRCASEEQ(",", formatChange.NumericSpec().ThousandsSeparator().GetOld().Value().c_str());
    EXPECT_STRCASEEQ(".", formatChange.NumericSpec().ThousandsSeparator().GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      05/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareEnumerationsIdentical)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    ECEnumerationP enumeration;
    m_firstSchema->CreateEnumeration(enumeration, "enum", PrimitiveType::PRIMITIVETYPE_String);
    auto firstEnum = m_firstSchema->GetEnumerationP("enum");
    ECEnumeratorP enumerator;
    firstEnum->CreateEnumerator(enumerator, "blah", "banana");
    m_secondSchema->CreateEnumeration(enumeration, "enum", PrimitiveType::PRIMITIVETYPE_String);
    auto secondEnum = m_secondSchema->GetEnumerationP("enum");
    secondEnum->CreateEnumerator(enumerator, "blah", "banana");
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);
    ASSERT_EQ(0, changes.Changes().Count());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      05/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareEnumerationsDeleted)
    {
    {
    CreateFirstSchema();
    CreateSecondSchema();

    ECEnumerationP enumeration;
    m_firstSchema->CreateEnumeration(enumeration, "enum", PrimitiveType::PRIMITIVETYPE_String);
    auto firstEnum = m_firstSchema->GetEnumerationP("enum");
    ECEnumeratorP enumerator;
    firstEnum->CreateEnumerator(enumerator, "blah", "banana");
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);
    ASSERT_EQ(1, changes.Changes().Count());
    auto& change = changes.Changes()[0].Enumerations();

    ASSERT_EQ(1, change.Count());
    auto enumChange = change[0];

    ASSERT_EQ(6, enumChange.MemberChangesCount());
    }

    {
    CreateFirstSchema();
    CreateSecondSchema();

    ECEnumerationP enumeration;
    m_secondSchema->CreateEnumeration(enumeration, "enum", PrimitiveType::PRIMITIVETYPE_String);
    auto firstEnum = m_secondSchema->GetEnumerationP("enum");
    ECEnumeratorP enumerator;
    firstEnum->CreateEnumerator(enumerator, "blah", "banana");
    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);
    ASSERT_EQ(1,  changes.Changes().Count());
    auto& change = changes.Changes()[0].Enumerations();

    ASSERT_EQ(1, change.Count());
    auto enumChange = change[0];

    ASSERT_EQ(6, enumChange.MemberChangesCount());
    }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      05/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareECClassIdentical)
    {
    CreateFirstSchema();
    CreateSecondSchema();

    ECEntityClassP c;
    m_firstSchema->CreateEntityClass(c, "awesome");
    auto firstClass = m_firstSchema->GetClassP("awesome");
    ECEnumerationP enumeration;
    m_firstSchema->CreateEnumeration(enumeration, "enum", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveECPropertyP prop;
    firstClass->CreateEnumerationProperty(prop, "enum", *enumeration);
    PrimitiveArrayECPropertyP arrayProp;
    firstClass->CreatePrimitiveArrayProperty(arrayProp, "name");
    firstClass->CreatePrimitiveProperty(prop, "prim");
    StructECPropertyP structProp;
    ECStructClassP structClass;
    m_firstSchema->CreateStructClass(structClass, "structCLass");
    firstClass->CreateStructProperty(structProp, "struct", *structClass);
    StructArrayECPropertyP structArrayProp;
    firstClass->CreateStructArrayProperty(structArrayProp, "structArrayProp", *structClass);
    m_secondSchema->CreateEntityClass(c, "awesome");
    firstClass = m_secondSchema->GetClassP("awesome");
    m_secondSchema->CreateEnumeration(enumeration, "enum", PrimitiveType::PRIMITIVETYPE_String);
    firstClass->CreateEnumerationProperty(prop, "enum", *enumeration);
    firstClass->CreatePrimitiveArrayProperty(arrayProp, "name");
    firstClass->CreatePrimitiveProperty(prop, "prim");
    m_secondSchema->CreateStructClass(structClass, "structCLass");
    firstClass->CreateStructProperty(structProp, "struct", *structClass);
    firstClass->CreateStructArrayProperty(structArrayProp, "structArrayProp", *structClass);

    SchemaComparer comparer;
    SchemaDiff changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);
    ASSERT_EQ(0, changes.Changes().Count());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
