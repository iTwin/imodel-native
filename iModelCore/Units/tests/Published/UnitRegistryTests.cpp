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

struct UnitRegistryTests : UnitsTestFixture { };

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddAndRetrieveConstant)
    {
    // Add constant
    UnitRegistry* registry = new UnitRegistry();

    PhenomenonCP phen = registry->LookupPhenomenon("LENGTH");
    ASSERT_NE(nullptr, phen) << "The Phenomenon 'Length' does not exist in the registry";
    UnitCP createdConstant = registry->AddConstant(phen->GetName().c_str(), "SI", "TestConstant", "NUMBER", 42.0);
    ASSERT_NE(nullptr, createdConstant);
    EXPECT_STRCASEEQ("TestConstant", createdConstant->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(registry->HasUnit("TestConstant"));

    UnitCP retreivedConstant = registry->LookupConstant("TestConstant");
    EXPECT_NE(nullptr, retreivedConstant);
    EXPECT_EQ(createdConstant, retreivedConstant);

    UnitCP retreivedConstantAsUnit = registry->LookupUnit("TestConstant");
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
            UnitCP unit = s_unitsContext->LookupUnit(unitName.c_str());
            if (nullptr != unit)
                {
                foundUnits.push_back(unitName);
                continue;
                }

            unit = s_unitsContext->LookupUnitUsingOldName(unitName.c_str());
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
    bvector<UnitCP> allUnits;
    s_unitsContext->AllUnits(allUnits);
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
    EXPECT_TRUE(s_unitsContext->HasUnit("M"));
    EXPECT_TRUE(s_unitsContext->HasUnit("S"));
    EXPECT_TRUE(s_unitsContext->HasUnit("K"));
    EXPECT_TRUE(s_unitsContext->HasUnit("DELTA_KELVIN"));
    EXPECT_TRUE(s_unitsContext->HasUnit("A"));
    EXPECT_TRUE(s_unitsContext->HasUnit("MOL"));
    EXPECT_TRUE(s_unitsContext->HasUnit("CD"));
    EXPECT_TRUE(s_unitsContext->HasUnit("RAD"));
    EXPECT_TRUE(s_unitsContext->HasUnit("STERAD"));
    EXPECT_TRUE(s_unitsContext->HasUnit("US$"));
    EXPECT_TRUE(s_unitsContext->HasUnit("PERSON"));
    EXPECT_TRUE(s_unitsContext->HasUnit("ONE"));

    EXPECT_TRUE(s_unitsContext->LookupUnit("M")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("S")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("K")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("DELTA_KELVIN")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("A")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("MOL")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("CD")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("RAD")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("STERAD")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("US$")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("PERSON")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupUnit("ONE")->IsBase());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBasePhenomenaAdded)
    {
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("LENGTH"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("MASS"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("TIME"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("TEMPERATURE"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("TEMPERATURE_CHANGE"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("CURRENT"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("MOLE"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("LUMINOSITY"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("ANGLE"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("SOLIDANGLE"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("CURRENCY"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("CAPITA"));
    EXPECT_TRUE(s_unitsContext->HasPhenomenon("NUMBER"));

    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("LENGTH")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("MASS")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("TIME")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("TEMPERATURE")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("TEMPERATURE_CHANGE")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("CURRENT")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("MOLE")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("LUMINOSITY")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("ANGLE")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("SOLIDANGLE")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("CURRENCY")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("CAPITA")->IsBase());
    EXPECT_TRUE(s_unitsContext->LookupPhenomenon("NUMBER")->IsBase());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAllBaseUnitSystemsAdded)
    {
    EXPECT_TRUE(s_unitsContext->HasSystem("SI"));
    EXPECT_TRUE(s_unitsContext->HasSystem("CGS"));
    EXPECT_TRUE(s_unitsContext->HasSystem("METRIC"));
    EXPECT_TRUE(s_unitsContext->HasSystem("IMPERIAL"));
    EXPECT_TRUE(s_unitsContext->HasSystem("MARITIME"));
    EXPECT_TRUE(s_unitsContext->HasSystem("USSURVEY"));
    EXPECT_TRUE(s_unitsContext->HasSystem("INDUSTRIAL"));
    EXPECT_TRUE(s_unitsContext->HasSystem("INTERNATIONAL"));
    EXPECT_TRUE(s_unitsContext->HasSystem("USCUSTOM"));
    EXPECT_TRUE(s_unitsContext->HasSystem("STATISTICS"));
    EXPECT_TRUE(s_unitsContext->HasSystem("FINANCE"));
    EXPECT_TRUE(s_unitsContext->HasSystem("CONSTANT"));
    EXPECT_TRUE(s_unitsContext->HasSystem("DUMMY"));

    bvector<UnitSystemCP> unitSystems;
    s_unitsContext->AllSystems(unitSystems);
    EXPECT_EQ(13, unitSystems.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingNewBasePhenomenonAndUnit)
    {
    UnitRegistry* registry = new UnitRegistry();

    PhenomenonCP sillyPhen = registry->AddPhenomenon("Silly", "Silly");
    ASSERT_NE(nullptr, sillyPhen);
    EXPECT_TRUE(sillyPhen->IsBase());

    UnitCP laughUnit = registry->AddUnit("Silly", "SI", "Laugh", "Laugh");
    ASSERT_NE(nullptr, laughUnit);
    EXPECT_TRUE(laughUnit->IsBase());

    UnitCP megaLaughUnit = registry->AddUnit("Silly", "METRIC", "Megalaugh", "[MEGA]*Laugh");
    ASSERT_NE(nullptr, megaLaughUnit);
    EXPECT_FALSE(megaLaughUnit->IsBase());

    Quantity laughs(42, *megaLaughUnit);
    EXPECT_EQ(42000000, laughs.ConvertTo(laughUnit).GetMagnitude());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddDummyUnit)
    {
    UnitRegistry* registry = new UnitRegistry();
    UnitCP testDummy = registry->AddDummyUnit(nullptr);
    EXPECT_EQ(nullptr, testDummy);
    testDummy = registry->AddDummyUnit("");
    EXPECT_EQ(nullptr, testDummy);
    testDummy = registry->AddDummyUnit("M");
    ASSERT_NE(nullptr, testDummy);
    EXPECT_STRCASEEQ("M", testDummy->GetName().c_str());
    testDummy = registry->AddDummyUnit("banana");
    ASSERT_NE(nullptr, testDummy);
    EXPECT_STRCASEEQ("DUMMY", testDummy->GetUnitSystem()->GetName().c_str());
    EXPECT_STRCASEEQ("DUMMY_banana", testDummy->GetPhenomenon()->GetName().c_str());
    EXPECT_STRCASEEQ("banana", testDummy->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemovePhenomenon)
    {
    UnitRegistry* registry = new UnitRegistry();
    PhenomenonCP testPhenom = registry->AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);

    PhenomenonCP removedPhenom = registry->RemovePhenomenon("TestPhenomenon");
    ASSERT_EQ(testPhenom, removedPhenom);
    ASSERT_FALSE(registry->HasPhenomenon("TestPhenomenon"));

    EXPECT_STREQ("TestPhenomenon", removedPhenom->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveUnitSystem)
    {
    UnitRegistry* registry = new UnitRegistry();
    UnitSystemCP testSystem = registry->AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);

    UnitSystemCP removedSystem = registry->RemoveSystem("TestSystem");
    ASSERT_EQ(testSystem, removedSystem);
    ASSERT_FALSE(registry->HasSystem("TestSystem"));

    EXPECT_STREQ("TestSystem", removedSystem->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveUnits)
    {
    UnitRegistry* registry = new UnitRegistry();
    UnitCP testUnit = registry->AddUnit("LENGTH", "SI", "banana", "M");
    ASSERT_NE(nullptr, testUnit);

    UnitCP removedUnit = registry->RemoveUnit("banana");
    ASSERT_EQ(testUnit, removedUnit);
    ASSERT_FALSE(registry->HasUnit("banana"));

    EXPECT_STREQ("banana", removedUnit->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveInvertedUnits)
    {
    UnitRegistry* registry = new UnitRegistry();
    UnitCP testUnit = registry->AddInvertedUnit("M", "banana", "SI");
    ASSERT_NE(nullptr, testUnit);

    UnitCP removedUnit = registry->RemoveInvertedUnit("banana");
    ASSERT_EQ(testUnit, removedUnit);
    ASSERT_FALSE(registry->HasUnit("banana"));

    EXPECT_STREQ("banana", removedUnit->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveConstant)
    {
    UnitRegistry* registry = new UnitRegistry();
    UnitCP testUnit = registry->AddConstant("LENGTH", "SI", "banana", "M", 1.0);
    ASSERT_NE(nullptr, testUnit);

    UnitCP removedUnit = registry->RemoveConstant("banana");
    ASSERT_EQ(testUnit, removedUnit);
    ASSERT_FALSE(registry->HasUnit("banana"));

    EXPECT_STREQ("banana", removedUnit->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddDuplicateItems)
    {
    UnitRegistry* registry = new UnitRegistry();

    // Add test setup, with a named thing of each item
    ASSERT_NE(nullptr, registry->AddSystem("TestSystem"));
    ASSERT_NE(nullptr, registry->AddPhenomenon("TestPhenomenon", "LENGTH"));
    ASSERT_NE(nullptr, registry->AddUnit("TestPhenomenon", "TestSystem", "TestUnit", "M"));
    ASSERT_NE(nullptr, registry->AddInvertedUnit("TestUnit", "TestInvertedUnit", "TestSystem"));
    ASSERT_NE(nullptr, registry->AddConstant("TestPhenomenon", "TestSystem", "TestConstant", "M", 10.0));

    // Take the names and attempt to add all types
    auto names = {"TestSystem", "TestPhenomenon", "TestUnit", "TestConstant", "TestInvertedUnit"};
    for(const auto& name : names)
        EXPECT_EQ(nullptr, registry->AddUnit("TestPhenomenon","TestSystem", name, "M"));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, registry->AddConstant("TestPhenomenon","TestSystem", name, "M", 10.0));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, registry->AddInvertedUnit("TestUnit", name, "TestSystem"));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, registry->AddSystem(name));

    for(const auto& name : names)
        EXPECT_EQ(nullptr, registry->AddPhenomenon(name, "LENGTH"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestCaseInsensitiveLookup)
    {
    UnitRegistry* registry = new UnitRegistry();

    UnitSystemCP testSystem = registry->AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);
    UnitSystemCP retrievedSystem = registry->LookupUnitSystem("testSYSTEM");
    ASSERT_EQ(retrievedSystem, testSystem);

    PhenomenonCP testPhenom = registry->AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);
    PhenomenonCP retrievedPhenomenon = registry->LookupPhenomenon("TESTPHENOMENOn");
    ASSERT_EQ(retrievedPhenomenon, testPhenom);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
#if !defined (BENTLEYCONFIG_OS_LINUX) // informed Kyle/Caleb/Gintaras 9/28/18
TEST_F(UnitRegistryTests, AllNewNamesMapToECNames)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(L"All3_1Names.csv");
    std::ifstream ifs(path.begin(), std::ifstream::in);
    std::string line;
    bvector<Utf8String> ignoredNames = {"CM/REVOLUTION", "FT/REVOLUTION", "IN/DEGREE", "IN/RAD", "IN/REVOLUTION", "M/DEGREE", "MM/RAD", "MM/REVOLUTION"};
    
    bool isIgnoredName;
    Utf8String parsedName;
    while (std::getline(ifs, line))
        {
        parsedName = line.c_str();
        parsedName.Trim();
        isIgnoredName = false;
        
        for(auto const& ignoredName : ignoredNames)
            {
            if(ignoredName.EqualsI(parsedName.c_str()))
                isIgnoredName = true;
            }
        if(!isIgnoredName)
            { 
            auto mapped = UnitNameMappings::TryGetECNameFromNewName(parsedName.c_str());
            if (nullptr == mapped)
                {
                ASSERT_TRUE(false) << "Unit with new Name " << parsedName << " not mapped to an ec Name";
                continue;
                }
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:DECA"))
                parsedName = "DECA";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:MEGAPASCAL"))
                parsedName = "MEGAPASCAL";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:KPF"))
                parsedName = "KPF";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:PERSON"))
                parsedName = "PERSON";
            
            auto roundtrippedName = UnitNameMappings::TryGetNewNameFromECName(mapped);
            if (nullptr == roundtrippedName)
                {
                ASSERT_TRUE(false) <<  "Can't get name from ecname for unit " << mapped;
                continue;
                }
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "DEKA"))
                roundtrippedName = "DECA";
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "N/SQ.MM"))
                roundtrippedName = "MEGAPASCAL";
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "KIPF"))
                roundtrippedName = "KPF";
            if (0 == BeStringUtilities::StricmpAscii(roundtrippedName, "CAPITA"))
                roundtrippedName = "PERSON";

            auto newUnit = s_unitsContext->LookupUnit(parsedName.c_str());
            auto ecUnit = s_unitsContext->LookupUnit(roundtrippedName);
            EXPECT_EQ(newUnit, ecUnit) << "Failed to find " << roundtrippedName;
            }
        }
    }
#endif

END_UNITS_UNITTESTS_NAMESPACE
