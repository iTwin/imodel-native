/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/UnitRegistryTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../UnitsTestsPch.h"
#include "../TestFixture/UnitsTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

struct UnitRegistrySingletonTests : UnitsTestFixture 
    {
    void SetUp() override
        {
        UnitsTestFixture::SetUp();
        // Set the singleton to nullptr
        UnitRegistry::Clear();
        }

    void TearDown() override
        {
        UnitsTestFixture::TearDown();
        UnitRegistry::Clear();
        }
    };

struct UnitRegistryTests : UnitsTestFixture 
{
    struct TestUnitSystem : UnitSystem
    {
    public:
        TestUnitSystem(Utf8CP name) : UnitSystem(name) {}
    };
    struct TestPhenomenon : Phenomenon
    {
    public:
        TestPhenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) :
            Phenomenon(name, definition, baseSymbol, id) {}
    };

    struct TestUnit : Unit
    {
    public:
        TestUnit(UnitSystemCR unitSystem, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant) :
            Unit(unitSystem, phenomenon, name, id, definition, baseSymbol, factor, offset, isConstant) { }
    };

    struct TestUnitLocater : IUnitLocater
    {
    private:
        bmap<Utf8String, TestUnit*> m_units;
        TestPhenomenon* m_phenomenon;
        TestUnitSystem* m_unitSystem;

        void Populate();

    protected:
        UnitP _LocateUnitP(Utf8CP name) const override;
        PhenomenonP _LocatePhenomenonP(Utf8CP name) const override {return 0 == strcmp(m_phenomenon->GetName(), name) ? m_phenomenon : nullptr;}
        UnitSystemP _LocateUnitSystemP(Utf8CP name) const override {return 0 == strcmp(m_unitSystem->GetName(), name) ? m_unitSystem : nullptr;}

    public:
        TestUnitLocater() {Populate();}
    };
};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistrySingletonTests, AddAndRetrieveConstant)
    {
    // Add constant
    PhenomenonCP phen = UnitRegistry::Instance().LookupPhenomenon("LENGTH");
    ASSERT_NE(nullptr, phen) << "The Phenomenon 'Length' does not exist in the registry";
    UnitCP createdConstant = UnitRegistry::Instance().AddConstant(phen->GetName(), "TestConstant", "ONE", 0);
    ASSERT_NE(nullptr, createdConstant);

    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("TestConstant"));

    UnitCP retreivedConstant = UnitRegistry::Instance().LookupConstant("TestConstant");
    EXPECT_NE(nullptr, retreivedConstant);
    EXPECT_EQ(createdConstant, retreivedConstant);

    UnitCP retreivedConstantAsUnit = UnitRegistry::Instance().LookupUnit("TestConstant");
    EXPECT_NE(nullptr, retreivedConstantAsUnit);
    EXPECT_EQ(createdConstant, retreivedConstantAsUnit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitRegistrySingletonTests, AllUnitsNeededForFirstReleaseExist)
    {
    bvector<Utf8String> missingUnits;
    bvector<Utf8String> foundUnits;
    bvector<Utf8String> foundMappedUnits;
    auto lineProcessor = [&missingUnits, &foundUnits, &foundMappedUnits] (bvector<Utf8String>& lines)
        {
        for (auto const& unitName : lines)
            {
            UnitCP unit = UnitRegistry::Instance().LookupUnit(unitName.c_str());
            if (nullptr != unit)
                {
                foundUnits.push_back(unitName);
                continue;
                }

            unit = UnitRegistry::Instance().LookupUnitUsingOldName(unitName.c_str());
            if (nullptr != unit)
                {
                foundMappedUnits.push_back(unitName);
                continue;
                }

            missingUnits.push_back(unitName);
            }
        };

    ReadConversionCsvFile(L"NeededUnits.csv", lineProcessor);

    if (missingUnits.size() != 0)
        {
        Utf8String missingString = BeStringUtilities::Join(missingUnits, ", ");
        EXPECT_EQ(0, missingUnits.size()) << "Some needed units were not found\n" << missingString.c_str();
        }
    ASSERT_NE(0, foundUnits.size()) << "No units were found";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitRegistryTests, TestEveryDefaultUnitIsAddedToItsPhenomenon)
    {
    UnitRegistry& hub = UnitRegistry::Instance();
    bvector<UnitCP> allUnits;
    hub.AllUnits(allUnits);
    for (auto const& unit : allUnits)
        {
        PhenomenonCP unitPhenomenon = unit->GetPhenomenon();
        ASSERT_NE(nullptr, unitPhenomenon) << "Unit " << unit->GetName() << " does not have phenomenon";
        auto it = find_if(unitPhenomenon->GetUnits().begin(), unitPhenomenon->GetUnits().end(),
                       [&unit] (UnitCP unitInPhenomenon) { return 0 == strcmp(unitInPhenomenon->GetName(), unit->GetName()); });
        
        T_Utf8StringVector unitNames;
        if (unitPhenomenon->GetUnits().end() == it)
            {
            for (auto const& phenUnit : unitPhenomenon->GetUnits())
                unitNames.push_back(phenUnit->GetName());
            }
        ASSERT_NE(unitPhenomenon->GetUnits().end(), it) << "Unit " << unit->GetName() << " is not registered with it's phenomenon: " << unitPhenomenon->GetName() << "Registered units are: " << BeStringUtilities::Join(unitNames, ", ");
        }
    }

//=======================================================================================
//! UnitRegistryTests
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
void UnitRegistryTests::TestUnitLocater::Populate()
    {
    m_unitSystem = new TestUnitSystem("Banana");
    m_phenomenon = new TestPhenomenon("TestPhen", "Def", 'D', 0);

    m_units.insert(bpair<Utf8String, TestUnit*>("TestUnit1", new TestUnit(*m_unitSystem, *m_phenomenon, "TestUnit1", 1, "Def1", ' ', 1, 0, false)));
    m_units.insert(bpair<Utf8String, TestUnit*>("TestUnit2", new TestUnit(*m_unitSystem, *m_phenomenon, "TestUnit2", 2, "Def2", ' ', 1, 0, false)));
    m_units.insert(bpair<Utf8String, TestUnit*>("TestUnit3", new TestUnit(*m_unitSystem, *m_phenomenon, "TestUnit3", 3, "Def3", ' ', 1, 0, false)));
    m_units.insert(bpair<Utf8String, TestUnit*>("TestUnit4", new TestUnit(*m_unitSystem, *m_phenomenon, "TestUnit4", 4, "Def4", ' ', 1, 0, false)));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitP UnitRegistryTests::TestUnitLocater::_LocateUnitP(Utf8CP name) const
    {
    auto val_iter = m_units.find(name);
    if (val_iter == m_units.end())
        return nullptr;

    return (*val_iter).second;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddUnitLocater)
    {
    TestUnitLocater* locater = new TestUnitLocater();

    UnitRegistryPtr registry = UnitRegistry::Create();
    ASSERT_TRUE(registry.IsValid());

    EXPECT_EQ(nullptr, registry->LookupUnit("TestUnit1"));
    EXPECT_EQ(nullptr, registry->LookupPhenomenon("TestPhen"));

    registry->AddLocater(*locater);

    PhenomenonCP locaterPhen = registry->LookupPhenomenon("TestPhen");
    EXPECT_NE(nullptr, locaterPhen);

    UnitCP locaterUnit = registry->LookupUnit("TestUnit1");
    EXPECT_NE(nullptr, locaterUnit);

    registry->RemoveLocater();
    EXPECT_EQ(nullptr, registry->LookupUnit("TestUnit1"));
    EXPECT_EQ(nullptr, registry->LookupPhenomenon("TestPhen"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBaseUnitsAdded)
    {
    UnitRegistryPtr registry = UnitRegistry::Create();
    EXPECT_TRUE(registry->HasUnit("M"));
    EXPECT_TRUE(registry->HasUnit("S"));
    EXPECT_TRUE(registry->HasUnit("K"));
    EXPECT_TRUE(registry->HasUnit("DELTA_KELVIN"));
    EXPECT_TRUE(registry->HasUnit("A"));
    EXPECT_TRUE(registry->HasUnit("MOL"));
    EXPECT_TRUE(registry->HasUnit("CD"));
    EXPECT_TRUE(registry->HasUnit("RAD"));
    EXPECT_TRUE(registry->HasUnit("STERAD"));
    EXPECT_TRUE(registry->HasUnit("US$"));
    EXPECT_TRUE(registry->HasUnit("PERSON"));
    EXPECT_TRUE(registry->HasUnit("ONE"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBasePhenomenaAdded)
    {
    UnitRegistryPtr registry = UnitRegistry::Create();
    EXPECT_TRUE(registry->HasPhenomena("LENGTH"));
    EXPECT_TRUE(registry->HasPhenomena("MASS"));
    EXPECT_TRUE(registry->HasPhenomena("TIME"));
    EXPECT_TRUE(registry->HasPhenomena("TEMPERATURE"));
    EXPECT_TRUE(registry->HasPhenomena("TEMPERATURE_CHANGE"));
    EXPECT_TRUE(registry->HasPhenomena("CURRENT"));
    EXPECT_TRUE(registry->HasPhenomena("MOLE"));
    EXPECT_TRUE(registry->HasPhenomena("LUMINOSITY"));
    EXPECT_TRUE(registry->HasPhenomena("ANGLE"));
    EXPECT_TRUE(registry->HasPhenomena("SOLIDANGLE"));
    EXPECT_TRUE(registry->HasPhenomena("FINANCE"));
    EXPECT_TRUE(registry->HasPhenomena("CAPITA"));
    EXPECT_TRUE(registry->HasPhenomena("ONE"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBaseUnitSystemsAdded)
    {
    UnitRegistryPtr registry = UnitRegistry::Create();
    EXPECT_TRUE(registry->HasSystem("SI"));
    EXPECT_TRUE(registry->HasSystem("CGS"));
    EXPECT_TRUE(registry->HasSystem("METRIC"));
    EXPECT_TRUE(registry->HasSystem("IMPERIAL"));
    EXPECT_TRUE(registry->HasSystem("MARITIME"));
    EXPECT_TRUE(registry->HasSystem("USSURVEY"));
    EXPECT_TRUE(registry->HasSystem("INDUSTRIAL"));
    EXPECT_TRUE(registry->HasSystem("INTERNATIONAL"));
    EXPECT_TRUE(registry->HasSystem("USCUSTOM"));
    EXPECT_TRUE(registry->HasSystem("STATISTICS"));
    EXPECT_TRUE(registry->HasSystem("FINANCE"));
    EXPECT_TRUE(registry->HasSystem("CONSTANT"));

    bvector<UnitSystemCP> unitSystems;
    registry->AllSystems(unitSystems);

    EXPECT_EQ(12, unitSystems.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestStaticBaseUnitSystemsAreOnlyCreatedOnce)
    {
    UnitRegistryPtr registry1 = UnitRegistry::Create();
    UnitRegistryPtr registry2 = UnitRegistry::Create();

    EXPECT_EQ(registry1->LookupUnitSystem("SI"), registry2->LookupUnitSystem("SI"));
    EXPECT_EQ(registry1->LookupUnitSystem("CGS"), registry2->LookupUnitSystem("CGS"));
    EXPECT_EQ(registry1->LookupUnitSystem("METRIC"), registry2->LookupUnitSystem("METRIC"));
    EXPECT_EQ(registry1->LookupUnitSystem("IMPERIAL"), registry2->LookupUnitSystem("IMPERIAL"));
    EXPECT_EQ(registry1->LookupUnitSystem("MARITIME"), registry2->LookupUnitSystem("MARITIME"));
    EXPECT_EQ(registry1->LookupUnitSystem("USSURVEY"), registry2->LookupUnitSystem("USSURVEY"));
    EXPECT_EQ(registry1->LookupUnitSystem("INDUSTRIAL"), registry2->LookupUnitSystem("INDUSTRIAL"));
    EXPECT_EQ(registry1->LookupUnitSystem("INTERNATIONAL"), registry2->LookupUnitSystem("INTERNATIONAL"));
    EXPECT_EQ(registry1->LookupUnitSystem("USCUSTOM"), registry2->LookupUnitSystem("USCUSTOM"));
    EXPECT_EQ(registry1->LookupUnitSystem("STATISTICS"), registry2->LookupUnitSystem("STATISTICS"));
    EXPECT_EQ(registry1->LookupUnitSystem("FINANCE"), registry2->LookupUnitSystem("FINANCE"));
    EXPECT_EQ(registry1->LookupUnitSystem("CONSTANT"), registry2->LookupUnitSystem("CONSTANT"));
    
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("SI"), registry2->LookupUnitSystem("SI"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("CGS"), registry2->LookupUnitSystem("CGS"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("METRIC"), registry2->LookupUnitSystem("METRIC"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("IMPERIAL"), registry2->LookupUnitSystem("IMPERIAL"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("MARITIME"), registry2->LookupUnitSystem("MARITIME"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("USSURVEY"), registry2->LookupUnitSystem("USSURVEY"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("INDUSTRIAL"), registry2->LookupUnitSystem("INDUSTRIAL"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("INTERNATIONAL"), registry2->LookupUnitSystem("INTERNATIONAL"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("USCUSTOM"), registry2->LookupUnitSystem("USCUSTOM"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("STATISTICS"), registry2->LookupUnitSystem("STATISTICS"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("FINANCE"), registry2->LookupUnitSystem("FINANCE"));
    EXPECT_EQ(UnitRegistry::Instance().LookupUnitSystem("CONSTANT"), registry2->LookupUnitSystem("CONSTANT"));
    }

END_UNITS_UNITTESTS_NAMESPACE
