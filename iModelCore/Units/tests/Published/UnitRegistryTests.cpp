/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/UnitRegistryTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/UnitsTestFixture.h"
#include <fstream>
BEGIN_UNITS_UNITTESTS_NAMESPACE

struct UnitRegistryTests : UnitsTestFixture
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

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddAndRetrieveConstant)
    {
    // Add constant
    PhenomenonCP phen = UnitRegistry::Get().LookupPhenomenon("LENGTH");
    ASSERT_NE(nullptr, phen) << "The Phenomenon 'Length' does not exist in the registry";
    UnitCP createdConstant = UnitRegistry::Get().AddConstant(phen->GetName().c_str(), "SI", "TestConstant", "NUMBER", 42.0);
    ASSERT_NE(nullptr, createdConstant);

    EXPECT_TRUE(UnitRegistry::Get().HasUnit("TestConstant"));

    UnitCP retreivedConstant = UnitRegistry::Get().LookupConstant("TestConstant");
    EXPECT_NE(nullptr, retreivedConstant);
    EXPECT_EQ(createdConstant, retreivedConstant);

    UnitCP retreivedConstantAsUnit = UnitRegistry::Get().LookupUnit("TestConstant");
    EXPECT_NE(nullptr, retreivedConstantAsUnit);
    EXPECT_EQ(createdConstant, retreivedConstantAsUnit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitRegistryTests, AllUnitsNeededForFirstReleaseExist)
    {
    bvector<Utf8String> missingUnits;
    bvector<Utf8String> foundUnits;
    bvector<Utf8String> foundMappedUnits;
    auto lineProcessor = [&missingUnits, &foundUnits, &foundMappedUnits] (bvector<Utf8String>& lines)
        {
        for (auto const& unitName : lines)
            {
            UnitCP unit = UnitRegistry::Get().LookupUnit(unitName.c_str());
            if (nullptr != unit)
                {
                foundUnits.push_back(unitName);
                continue;
                }

            unit = UnitRegistry::Get().LookupUnitUsingOldName(unitName.c_str());
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
    UnitRegistry& hub = UnitRegistry::Get();
    bvector<UnitCP> allUnits;
    hub.AllUnits(allUnits);
    for (auto const& unit : allUnits)
        {
        PhenomenonCP unitPhenomenon = unit->GetPhenomenon();
        ASSERT_NE(nullptr, unitPhenomenon) << "Unit " << unit->GetName() << " does not have phenomenon";
        auto it = find_if(unitPhenomenon->GetUnits().begin(), unitPhenomenon->GetUnits().end(),
                       [&unit] (UnitCP unitInPhenomenon) { return 0 == strcmp(unitInPhenomenon->GetName().c_str(), unit->GetName().c_str()); });
        
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
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("M"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("S"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("K"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("DELTA_KELVIN"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("A"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("MOL"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("CD"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("RAD"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("STERAD"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("US$"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("PERSON"));
    EXPECT_TRUE(UnitRegistry::Get().HasUnit("ONE"));

    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("M")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("S")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("K")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("DELTA_KELVIN")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("A")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("MOL")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("CD")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("RAD")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("STERAD")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("US$")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("PERSON")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupUnit("ONE")->IsBase());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBasePhenomenaAdded)
    {
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("LENGTH"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("MASS"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("TIME"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("TEMPERATURE"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("TEMPERATURE_CHANGE"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("CURRENT"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("MOLE"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("LUMINOSITY"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("ANGLE"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("SOLIDANGLE"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("CURRENCY"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("CAPITA"));
    EXPECT_TRUE(UnitRegistry::Get().HasPhenomenon("NUMBER"));

    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("LENGTH")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("MASS")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("TIME")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("TEMPERATURE")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("TEMPERATURE_CHANGE")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("CURRENT")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("MOLE")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("LUMINOSITY")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("ANGLE")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("SOLIDANGLE")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("CURRENCY")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("CAPITA")->IsBase());
    EXPECT_TRUE(UnitRegistry::Get().LookupPhenomenon("NUMBER")->IsBase());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBaseUnitSystemsAdded)
    {
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("SI"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("CGS"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("METRIC"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("IMPERIAL"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("MARITIME"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("USSURVEY"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("INDUSTRIAL"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("INTERNATIONAL"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("USCUSTOM"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("STATISTICS"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("FINANCE"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("CONSTANT"));
    EXPECT_TRUE(UnitRegistry::Get().HasSystem("DUMMY"));

    bvector<UnitSystemCP> unitSystems;
    UnitRegistry::Get().AllSystems(unitSystems);
    EXPECT_EQ(13, unitSystems.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingNewBasePhenomenonAndUnit)
    {
    PhenomenonCP sillyPhen = UnitRegistry::Get().AddPhenomenon("Silly", "Silly");
    ASSERT_NE(nullptr, sillyPhen);
    EXPECT_TRUE(sillyPhen->IsBase());

    UnitCP laughUnit = UnitRegistry::Get().AddUnit("Silly", "SI", "Laugh", "Laugh");
    ASSERT_NE(nullptr, laughUnit);
    EXPECT_TRUE(laughUnit->IsBase());

    UnitCP megaLaughUnit = UnitRegistry::Get().AddUnit("Silly", "METRIC", "Megalaugh", "[MEGA]*Laugh");
    ASSERT_NE(nullptr, megaLaughUnit);
    EXPECT_FALSE(megaLaughUnit->IsBase());

    Quantity laughs(42, *megaLaughUnit);
    EXPECT_EQ(42000000, laughs.ConvertTo(laughUnit).GetMagnitude());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemovePhenomenon)
    {
    PhenomenonCP testPhenom = UnitRegistry::Get().AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);

    PhenomenonCP removedPhenom = UnitRegistry::Get().RemovePhenomenon("TestPhenomenon");
    ASSERT_EQ(testPhenom, removedPhenom);
    ASSERT_FALSE(UnitRegistry::Get().HasPhenomenon("TestPhenomenon"));

    EXPECT_STREQ("TestPhenomenon", removedPhenom->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveUnitSystem)
    {
    UnitSystemCP testSystem = UnitRegistry::Get().AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);

    UnitSystemCP removedSystem = UnitRegistry::Get().RemoveSystem("TestSystem");
    ASSERT_EQ(testSystem, removedSystem);
    ASSERT_FALSE(UnitRegistry::Get().HasSystem("TestSystem"));

    EXPECT_STREQ("TestSystem", removedSystem->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddDuplicateItems)
    {
    // Add test setup, with a named thing of each item
    ASSERT_NE(nullptr, UnitRegistry::Get().AddSystem("TestSystem"));
    ASSERT_NE(nullptr, UnitRegistry::Get().AddPhenomenon("TestPhenomenon", "LENGTH"));
    ASSERT_NE(nullptr, UnitRegistry::Get().AddUnit("TestPhenomenon", "TestSystem", "TestUnit", "M"));
    ASSERT_NE(nullptr, UnitRegistry::Get().AddInvertedUnit("TestUnit", "TestInvertedUnit", "TestSystem"));
    ASSERT_NE(nullptr, UnitRegistry::Get().AddConstant("TestPhenomenon", "TestSystem", "TestConstant", "M", 10.0));

    // Take the names and attempt to add all types
    auto names = {"TestSystem", "TestPhenomenon", "TestUnit", "TestConstant", "TestInvertedUnit"};
    for(const auto& name : names)
        EXPECT_EQ(nullptr, UnitRegistry::Get().AddUnit("TestPhenomenon","TestSystem", name, "M"));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, UnitRegistry::Get().AddConstant("TestPhenomenon","TestSystem", name, "M", 10.0));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, UnitRegistry::Get().AddInvertedUnit("TestUnit", name, "TestSystem"));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, UnitRegistry::Get().AddSystem(name));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, UnitRegistry::Get().AddPhenomenon(name, "LENGTH"));
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestCaseInsensitiveLookup)
    {
    UnitSystemCP testSystem = UnitRegistry::Get().AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);
    UnitSystemCP retrievedSystem = UnitRegistry::Get().LookupUnitSystem("testSYSTEM");
    ASSERT_EQ(retrievedSystem, testSystem);

    PhenomenonCP testPhenom = UnitRegistry::Get().AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);
    PhenomenonCP retrievedPhenomenon = UnitRegistry::Get().LookupPhenomenon("TESTPHENOMENOn");
    ASSERT_EQ(retrievedPhenomenon, testPhenom);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AllNewNamesMapToECNames)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(L"All3_1Names.csv");
    std::ifstream ifs(path.begin(), std::ifstream::in);
    std::string line;
    bvector<Utf8String> ignoredNames = {"CM/REVOLUTION", "FT/REVOLUTION", "IN/DEGREE", "IN/RAD", "IN/REVOLUTION", "M/DEGREE", "M/RAD", "M/REVOLUTION", "MM/RAD", "MM/REVOLUTION"};
    
    bool ignore = false;
    while (std::getline(ifs, line))
        {
        for(auto const& name : ignoredNames)
            {
            if(name.EqualsI(line.c_str()))
                ignore=true;
            }
        if(!ignore)
            { 
            auto mapped = UnitNameMappings::TryGetECNameFromNewName(line.c_str());
            EXPECT_NE(nullptr, mapped) << "Unit with new Name " << line << " not mapped to an ec Name";
            if(0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:DECA"))
                line = "DECA";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:MEGAPASCAL"))
                line = "MEGAPASCAL";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:KPF"))
                line = "KPF";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:PERSON"))
                line = "PERSON";
            auto newUnit = UnitRegistry::Get().LookupUnit(line.c_str());
            auto roundtrippedName = UnitNameMappings::TryGetNewNameFromECName(mapped);
            EXPECT_NE(nullptr, roundtrippedName) << "Can't get name from ecname for unit " << mapped;
            if(0 == BeStringUtilities::StricmpAscii(roundtrippedName, "DEKA"))
                roundtrippedName = "DECA";
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "N/SQ.MM"))
                roundtrippedName = "MEGAPASCAL";
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "KIPF"))
                roundtrippedName = "KPF";
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "CAPITA"))
                roundtrippedName = "PERSON";
            auto ecUnit = UnitRegistry::Get().LookupUnit(roundtrippedName);
            EXPECT_EQ(newUnit, ecUnit) << "Failed to find " << roundtrippedName;
            }
        ignore=false;
        }
    }

END_UNITS_UNITTESTS_NAMESPACE
