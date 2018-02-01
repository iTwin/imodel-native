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
TEST_F(UnitRegistryTests, TestAllBaseUnitsAdded)
    {
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("M"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("S"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("K"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("DELTA_KELVIN"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("A"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("MOL"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("CD"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("RAD"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("STERAD"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("US$"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("PERSON"));
    EXPECT_TRUE(UnitRegistry::Instance().HasUnit("ONE"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBasePhenomenaAdded)
    {
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("LENGTH"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("MASS"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("TIME"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("TEMPERATURE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("TEMPERATURE_CHANGE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("CURRENT"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("MOLE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("LUMINOSITY"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("ANGLE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("SOLIDANGLE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("FINANCE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("CAPITA"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomena("ONE"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBaseUnitSystemsAdded)
    {
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("SI"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("CGS"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("METRIC"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("IMPERIAL"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("MARITIME"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("USSURVEY"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("INDUSTRIAL"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("INTERNATIONAL"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("USCUSTOM"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("STATISTICS"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("FINANCE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasSystem("CONSTANT"));

    bvector<UnitSystemCP> unitSystems;
    UnitRegistry::Instance().AllSystems(unitSystems);
    EXPECT_EQ(12, unitSystems.size());
    }

END_UNITS_UNITTESTS_NAMESPACE
