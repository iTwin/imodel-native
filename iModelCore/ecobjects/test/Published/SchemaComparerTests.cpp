/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaComparerTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(0, changes.Count()); //Identical
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());

    auto& firstChanges = changes.At(0);

    ASSERT_EQ(3, firstChanges.ChangesCount());

    auto& unitChanges = firstChanges.Units();
    auto& phenomChanges = firstChanges.Phenomena();
    auto& systemChanges = firstChanges.UnitSystems();

    ASSERT_EQ(1, unitChanges.Count());
    ASSERT_EQ(1, phenomChanges.Count());
    ASSERT_EQ(1, systemChanges.Count());

    EXPECT_STREQ("SMOOT", unitChanges.At(0).GetDefinition().GetOld().Value().c_str());
    EXPECT_STREQ("APPLE", unitChanges.At(0).GetDefinition().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT", unitChanges.At(0).GetDisplayLabel().GetOld().Value().c_str());
    EXPECT_STREQ("BANANA", unitChanges.At(0).GetDisplayLabel().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT", unitChanges.At(0).GetDescription().GetOld().Value().c_str());
    EXPECT_STREQ("PEAR", unitChanges.At(0).GetDescription().GetNew().Value().c_str());

    EXPECT_STREQ("SMOOT", phenomChanges.At(0).GetDefinition().GetOld().Value().c_str());
    EXPECT_STREQ("APPLE", phenomChanges.At(0).GetDefinition().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT_PHENOM_LABEL", phenomChanges.At(0).GetDisplayLabel().GetOld().Value().c_str());
    EXPECT_STREQ("BANANA", phenomChanges.At(0).GetDisplayLabel().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT_PHENOM_DESCRIPTION", phenomChanges.At(0).GetDescription().GetOld().Value().c_str());
    EXPECT_STREQ("PEAR", phenomChanges.At(0).GetDescription().GetNew().Value().c_str());

    EXPECT_STREQ("SMOOT_SYSTEM_LABEL", systemChanges.At(0).GetDisplayLabel().GetOld().Value().c_str());
    EXPECT_STREQ("APPLE", systemChanges.At(0).GetDisplayLabel().GetNew().Value().c_str());
    EXPECT_STREQ("SMOOT_SYSTEM_DESCRIPTION", systemChanges.At(0).GetDescription().GetOld().Value().c_str());
    EXPECT_STREQ("BANANA", systemChanges.At(0).GetDescription().GetNew().Value().c_str());
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());

    auto& firstChanges = changes.At(0);

    ASSERT_EQ(4, firstChanges.ChangesCount());

    auto& unitChanges = firstChanges.Units();

    ASSERT_EQ(1, unitChanges.Count());
    
    EXPECT_STRCASEEQ("TestSchema:SMOOT_PHENOM", unitChanges.At(0).GetPhenomenon().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:SMOOT_PHENOM", unitChanges.At(0).GetPhenomenon().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("TestSchema:SMOOT_SYSTEM", unitChanges.At(0).GetUnitSystem().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:SMOOT_SYSTEM", unitChanges.At(0).GetUnitSystem().GetNew().Value().c_str());
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
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    m_secondSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"));

    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(0, changes.Count()); //Identical
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
    EC_ASSERT_SUCCESS(ref->CreateFormat(format, "Feet4U", nullptr, nullptr, &num));

    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("Feet4U"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    koq->AddPresentationFormat(*format);

    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());

    auto& koqChanges = changes.At(0).KindOfQuantities();

    ASSERT_EQ(1, changes.Count());

    auto& pers = koqChanges.At(0).GetPersistenceUnit();
    auto& pres = koqChanges.At(0).GetPresentationUnitList();

    EXPECT_STRCASEEQ("Units:M", pers.GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:M", pers.GetNew().Value().c_str());

    ASSERT_EQ(1, pres.Count());

    EXPECT_FALSE(pres.At(0).GetOld().IsNull());
    EXPECT_STRCASEEQ("f:Feet4U", pres.At(0).GetOld().Value().c_str());

    EXPECT_FALSE(pres.At(0).GetNew().IsNull());
    EXPECT_STRCASEEQ("r:Feet4U", pres.At(0).GetNew().Value().c_str());
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
    EC_ASSERT_SUCCESS(koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU")));
    ASSERT_EQ(1, koq->GetPresentationFormats().size());

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    EC_ASSERT_SUCCESS(m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    EC_ASSERT_SUCCESS(m_secondSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema()));
    ASSERT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EC_ASSERT_SUCCESS(koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("Feet4U")));
    ASSERT_EQ(1, koq->GetPresentationFormats().size());

    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    ASSERT_EQ(1, changes.At(0).ChangesCount());
    auto& koqChanges = changes.At(0).KindOfQuantities();

    ASSERT_EQ(1, koqChanges.Count());

    EXPECT_STRCASEEQ("Units:CM", koqChanges.At(0).GetPersistenceUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Units:M", koqChanges.At(0).GetPersistenceUnit().GetNew().Value().c_str());

    auto& pres = koqChanges.At(0).GetPresentationUnitList();
    ASSERT_EQ(1, pres.Count());

    ASSERT_FALSE(pres.At(0).GetOld().IsNull());
    EXPECT_STRCASEEQ("f:InchesU", pres.At(0).GetOld().Value().c_str());

    ASSERT_FALSE(pres.At(0).GetNew().IsNull());
    EXPECT_STRCASEEQ("f:Feet4U", pres.At(0).GetNew().Value().c_str());
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

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetPrefixPadChar('0');
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num, &comp);
    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(0, changes.Count()); //Identical
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& formatChanges = changes.At(0).Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges.At(0);

    ASSERT_EQ(1, formatChange.ChangesCount());

    EXPECT_TRUE(formatChange.GetHasComposite().GetOld().Value());
    EXPECT_FALSE(formatChange.GetHasComposite().GetNew().Value());
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& formatChanges = changes.At(0).Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges.At(0);

    ASSERT_EQ(8, formatChange.ChangesCount());

    EXPECT_STRCASEEQ("UNITS:DM", formatChange.GetCompositeMiddleUnit().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMiddleUnit().GetNew().IsNull());
    EXPECT_STRCASEEQ("UNITS:CM", formatChange.GetCompositeMinorUnit().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMinorUnit().GetNew().IsNull());
    EXPECT_STRCASEEQ("UNITS:MM", formatChange.GetCompositeSubUnit().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeSubUnit().GetNew().IsNull());

    EXPECT_STRCASEEQ("Spacer1", formatChange.GetCompositeSpacer().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeSpacer().GetNew().IsNull());

    EXPECT_STRCASEEQ("Apple", formatChange.GetCompositeMajorLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMajorLabel().GetNew().IsNull());
    EXPECT_STRCASEEQ("Banana", formatChange.GetCompositeMiddleLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMiddleLabel().GetNew().IsNull());
    EXPECT_STRCASEEQ("Carrot", formatChange.GetCompositeMinorLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMinorLabel().GetNew().IsNull());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.GetCompositeSubLabel().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeSubLabel().GetNew().IsNull());
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& formatChanges = changes.At(0).Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges.At(0);

    ASSERT_EQ(8, formatChange.ChangesCount());

    EXPECT_STRCASEEQ("UNITS:DM", formatChange.GetCompositeMiddleUnit().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMiddleUnit().GetOld().IsNull());
    EXPECT_STRCASEEQ("UNITS:CM", formatChange.GetCompositeMinorUnit().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMinorUnit().GetOld().IsNull());
    EXPECT_STRCASEEQ("UNITS:MM", formatChange.GetCompositeSubUnit().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeSubUnit().GetOld().IsNull());

    EXPECT_STRCASEEQ("Spacer1", formatChange.GetCompositeSpacer().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeSpacer().GetOld().IsNull());

    EXPECT_STRCASEEQ("Apple", formatChange.GetCompositeMajorLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMajorLabel().GetOld().IsNull());
    EXPECT_STRCASEEQ("Banana", formatChange.GetCompositeMiddleLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMiddleLabel().GetOld().IsNull());
    EXPECT_STRCASEEQ("Carrot", formatChange.GetCompositeMinorLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeMinorLabel().GetOld().IsNull());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.GetCompositeSubLabel().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetCompositeSubLabel().GetOld().IsNull());
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
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& formatChanges = changes.At(0).Formats();

    ASSERT_EQ(1, formatChanges.Count());
    auto& formatChange = formatChanges.At(0);

    ASSERT_EQ(9, formatChange.ChangesCount());

    EXPECT_STRCASEEQ("UNITS:M", formatChange.GetCompositeMajorUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:MILE", formatChange.GetCompositeMajorUnit().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:DM", formatChange.GetCompositeMiddleUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:YRD", formatChange.GetCompositeMiddleUnit().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:CM", formatChange.GetCompositeMinorUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:FT", formatChange.GetCompositeMinorUnit().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:MM", formatChange.GetCompositeSubUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("UNITS:IN", formatChange.GetCompositeSubUnit().GetNew().Value().c_str());

    EXPECT_STRCASEEQ("Spacer1", formatChange.GetCompositeSpacer().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Spacer2", formatChange.GetCompositeSpacer().GetNew().Value().c_str());

    EXPECT_STRCASEEQ("Apple", formatChange.GetCompositeMajorLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.GetCompositeMajorLabel().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Banana", formatChange.GetCompositeMiddleLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Eggplant", formatChange.GetCompositeMiddleLabel().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Carrot", formatChange.GetCompositeMinorLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Fig", formatChange.GetCompositeMinorLabel().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Dragonfruit", formatChange.GetCompositeSubLabel().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Grapefruit", formatChange.GetCompositeSubLabel().GetNew().Value().c_str());
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
    num.SetPrefixPadChar('0');
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana");
    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& change = changes.At(0).Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change.At(0);

    EXPECT_TRUE(formatChange.GetHasNumeric().GetOld().Value());
    EXPECT_FALSE(formatChange.GetHasNumeric().GetNew().Value());
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
    num.SetPrefixPadChar('0');
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    auto num2 = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num2);
    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& change = changes.At(0).Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change.At(0);

    ASSERT_EQ(9, formatChange.ChangesCount());

    EXPECT_STRCASEEQ("applyRounding", formatChange.GetFormatTraits().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetFormatTraits().GetNew().IsNull());
    EXPECT_STRCASEEQ("Fractional", formatChange.GetPresentationType().GetOld().Value().c_str());
    EXPECT_EQ(128, formatChange.GetFractionalPrecision().GetOld().Value());
    EXPECT_EQ(4, formatChange.GetMinWidth().GetOld().Value());
    EXPECT_TRUE(formatChange.GetMinWidth().GetNew().IsNull());
    EXPECT_STRCASEEQ("", formatChange.GetUomSeparator().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetUomSeparator().GetNew().IsNull());
    EXPECT_STRCASEEQ("0", formatChange.GetPrefixPadChar().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetPrefixPadChar().GetNew().IsNull());
    EXPECT_STRCASEEQ("NegativeParentheses", formatChange.GetShowSignOption().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetShowSignOption().GetNew().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.GetDecimalSeparator().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetDecimalSeparator().GetNew().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.GetThousandsSeparator().GetOld().Value().c_str());
    EXPECT_TRUE(formatChange.GetThousandsSeparator().GetNew().IsNull());
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
    num.SetPrefixPadChar('0');
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    auto num2 = Formatting::NumericFormatSpec();

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num2);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& change = changes.At(0).Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change.At(0);

    ASSERT_EQ(9, formatChange.ChangesCount());

    EXPECT_STRCASEEQ("applyRounding", formatChange.GetFormatTraits().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetFormatTraits().GetOld().IsNull());
    EXPECT_STRCASEEQ("Fractional", formatChange.GetPresentationType().GetNew().Value().c_str());
    EXPECT_EQ(128, formatChange.GetFractionalPrecision().GetNew().Value());
    EXPECT_EQ(4, formatChange.GetMinWidth().GetNew().Value());
    EXPECT_TRUE(formatChange.GetMinWidth().GetOld().IsNull());
    EXPECT_STRCASEEQ("", formatChange.GetUomSeparator().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetUomSeparator().GetOld().IsNull());
    EXPECT_STRCASEEQ("0", formatChange.GetPrefixPadChar().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetPrefixPadChar().GetOld().IsNull());
    EXPECT_STRCASEEQ("NegativeParentheses", formatChange.GetShowSignOption().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetShowSignOption().GetOld().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.GetDecimalSeparator().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetDecimalSeparator().GetOld().IsNull());
    EXPECT_STRCASEEQ(",", formatChange.GetThousandsSeparator().GetNew().Value().c_str());
    EXPECT_TRUE(formatChange.GetThousandsSeparator().GetOld().IsNull());
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
    num.SetPrefixPadChar('0');
    num.SetSignOption(Formatting::SignOption::NegativeParentheses);
    num.SetDecimalSeparator(',');
    num.SetThousandSeparator(',');

    auto num2 = Formatting::NumericFormatSpec();
    num2.SetExponentOnlyNegative(true);
    num2.SetPresentationType(Formatting::PresentationType::Decimal);
    num2.SetPrecision(Formatting::DecimalPrecision::Precision10);
    num2.SetMinWidth(8);
    num2.SetUomSeparator("smoot");
    num2.SetPrefixPadChar('x');
    num2.SetSignOption(Formatting::SignOption::OnlyNegative);
    num2.SetDecimalSeparator('.');
    num2.SetThousandSeparator('.');

    ECFormatP format;
    m_firstSchema->CreateFormat(format, "foo", "apple", "banana", &num);
    m_secondSchema->CreateFormat(format, "foo", "apple", "banana", &num2);
    SchemaComparer comparer;
    SchemaChanges changes;
    bvector<ECSchemaCP> first;
    bvector<ECSchemaCP> second;
    first.push_back(m_firstSchema.get());
    second.push_back(m_secondSchema.get());
    comparer.Compare(changes, first, second);

    ASSERT_EQ(1, changes.Count());
    auto& change = changes.At(0).Formats();

    ASSERT_EQ(1, change.Count());
    auto formatChange = change.At(0);

    ASSERT_EQ(10, formatChange.ChangesCount());

    EXPECT_STRCASEEQ("applyRounding", formatChange.GetFormatTraits().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("exponentOnlyNegative", formatChange.GetFormatTraits().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("Fractional", formatChange.GetPresentationType().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Decimal", formatChange.GetPresentationType().GetNew().Value().c_str());
    EXPECT_EQ(128, formatChange.GetFractionalPrecision().GetOld().Value());
    EXPECT_EQ(64, formatChange.GetFractionalPrecision().GetNew().Value());
    EXPECT_EQ(6, formatChange.GetDecimalPrecision().GetOld().Value());
    EXPECT_EQ(10, formatChange.GetDecimalPrecision().GetNew().Value());
    EXPECT_EQ(4, formatChange.GetMinWidth().GetOld().Value());
    EXPECT_EQ(8, formatChange.GetMinWidth().GetNew().Value());
    EXPECT_STRCASEEQ("", formatChange.GetUomSeparator().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("smoot", formatChange.GetUomSeparator().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("0", formatChange.GetPrefixPadChar().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("x", formatChange.GetPrefixPadChar().GetNew().Value().c_str());
    EXPECT_STRCASEEQ("NegativeParentheses", formatChange.GetShowSignOption().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("OnlyNegative", formatChange.GetShowSignOption().GetNew().Value().c_str());
    EXPECT_STRCASEEQ(",", formatChange.GetDecimalSeparator().GetOld().Value().c_str());
    EXPECT_STRCASEEQ(".", formatChange.GetDecimalSeparator().GetNew().Value().c_str());
    EXPECT_STRCASEEQ(",", formatChange.GetThousandsSeparator().GetOld().Value().c_str());
    EXPECT_STRCASEEQ(".", formatChange.GetThousandsSeparator().GetNew().Value().c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
