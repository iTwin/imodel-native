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
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
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
    EXPECT_STRCASEEQ("Formats:Feet4U", pres.At(0).GetOld().Value().c_str());

    EXPECT_FALSE(pres.At(0).GetNew().IsNull());
    EXPECT_STRCASEEQ("Ref:Feet4U", pres.At(0).GetNew().Value().c_str());
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
    ASSERT_EQ(ECObjectsStatus::Success,koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM")));
    ECTestFixture::GetFormatsSchema();
    EC_ASSERT_SUCCESS(koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("InchesU")));
    ASSERT_EQ(1, koq->GetPresentationFormatList().size());

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    EC_ASSERT_SUCCESS(m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    ASSERT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EC_ASSERT_SUCCESS(koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("Feet4U")));
    ASSERT_EQ(1, koq->GetPresentationFormatList().size());

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
    EXPECT_STRCASEEQ("Formats:InchesU", pres.At(0).GetOld().Value().c_str());

    ASSERT_FALSE(pres.At(0).GetNew().IsNull());
    EXPECT_STRCASEEQ("Formats:Feet4U", pres.At(0).GetNew().Value().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                      03/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(SchemaCompareTest, CompareFormatsIdentical)
    {
    CreateFirstSchema();
    CreateSecondSchema();
    auto comp = Formatting::CompositeValueSpec(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("DM"),*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"), *ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    comp.SetInputUnit(ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    comp.SetMajorLabel("Apple");
    comp.SetMiddleLabel("Bannana");
    comp.SetMinorLabel("Carrot");
    comp.SetSubLabel("Dragonfruit");

    auto num = Formatting::NumericFormatSpec();
    num.SetApplyRounding(true);
    num.SetPresentationType(Formatting::PresentationType::Fractional);
    num.SetFractionalPrecision(Formatting::FractionalPrecision::Over_128);
    num.SetMinWidth(4);
    num.SetUomSeparator("");
    num.SetPrefixPadChar('0');
    num.SetSignOption(Formatting::ShowSignOption::NegativeParentheses);
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

END_BENTLEY_ECN_TEST_NAMESPACE
