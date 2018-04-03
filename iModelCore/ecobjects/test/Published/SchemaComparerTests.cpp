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
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    // TODO
    //koq->AddPresentationUnit(*unit);

    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    // TODO
    //koq->AddPresentationUnit(*unit);

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
    // TODO
    //koq->AddPresentationUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("FT"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    // TODO
    // koq->AddPresentationUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("FT"));

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
    ECUnitP unit2;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(ref->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(ref->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    m_secondSchema->AddReferencedSchema(*ref);
    EC_ASSERT_SUCCESS(ref->CreateUnit(unit, "M", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(ref->CreateUnit(unit2, "FT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));

    EC_ASSERT_SUCCESS(m_firstSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    m_firstSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    // TODO
    // koq->AddPresentationUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("FT"));

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    koq->SetPersistenceUnit(*unit);
    // TODO
    // koq->AddPresentationUnit(*unit2);

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

    EXPECT_STRCASEEQ("Units:M(DefaultRealU)", pers.GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Ref:M(DefaultRealU)", pers.GetNew().Value().c_str());

    ASSERT_EQ(1, pres.Count());

    EXPECT_FALSE(pres.At(0).GetOld().IsNull());
    EXPECT_STRCASEEQ("Units:FT(DefaultRealU)", pres.At(0).GetOld().Value().c_str());

    EXPECT_FALSE(pres.At(0).GetNew().IsNull());
    EXPECT_STRCASEEQ("Ref:FT(DefaultRealU)", pres.At(0).GetNew().Value().c_str());
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
    // TODO
    //EC_ASSERT_SUCCESS(koq->AddPresentationUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("MM")));
    //ASSERT_EQ(1, koq->GetPresentationUnitList().size());

    EC_ASSERT_SUCCESS(m_secondSchema->CreateKindOfQuantity(koq, "KindOfSmoot"));
    EC_ASSERT_SUCCESS(m_secondSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema()));
    ASSERT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    // TODO
    //EC_ASSERT_SUCCESS(koq->AddPresentationUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("FT")));
    //ASSERT_EQ(1, koq->GetPresentationUnitList().size());

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

    EXPECT_STRCASEEQ("Units:CM(DefaultRealU)", koqChanges.At(0).GetPersistenceUnit().GetOld().Value().c_str());
    EXPECT_STRCASEEQ("Units:M(DefaultRealU)", koqChanges.At(0).GetPersistenceUnit().GetNew().Value().c_str());

    auto& pres = koqChanges.At(0).GetPresentationUnitList();
    ASSERT_EQ(1, pres.Count());

    ASSERT_FALSE(pres.At(0).GetOld().IsNull());
    EXPECT_STRCASEEQ("Units:MM(DefaultRealU)", pres.At(0).GetOld().Value().c_str());

    ASSERT_FALSE(pres.At(0).GetNew().IsNull());
    EXPECT_STRCASEEQ("Units:FT(DefaultRealU)", pres.At(0).GetNew().Value().c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
