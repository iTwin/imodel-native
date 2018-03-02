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

    struct TestUnitSystem : UnitSystem
        {
        friend struct UnitRegistry;
        private:
            TestUnitSystem(Utf8CP name) : UnitSystem(name) {}
        public:
            static TestUnitSystem* _Create(Utf8CP name) {return new TestUnitSystem(name);}
        };

    struct TestPhenomenon : Phenomenon
        {
        friend struct UnitRegistry;
        public:
            TestPhenomenon(Utf8CP name, Utf8CP definition) : Phenomenon(name, definition) {}
        public:
            static TestPhenomenon* _Create(Utf8CP name, Utf8CP definition) {return new TestPhenomenon(name, definition);}
        };

    struct TestUnit : Unit
        {
        friend struct UnitRegistry;
        private:
            TestUnit(UnitSystemCR unitSystem, PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant) :
                Unit(unitSystem, phenomenon, name, definition, numerator, denominator, offset, isConstant) {}

            TestUnit(UnitCR parentUnit, UnitSystemCR system, Utf8CP name)
            : TestUnit(system, *(parentUnit.GetPhenomenon()), name, parentUnit.GetDefinition().c_str(), 0, 0, 0, false) {}

        public:
            static TestUnit* _Create(UnitSystemCR sysName, PhenomenonCR phenomenon, Utf8CP unitName, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant)
            {return new TestUnit(sysName, phenomenon, unitName, definition, numerator, denominator, offset, isConstant);}

            static TestUnit* _Create(UnitCR parentUnit, UnitSystemCR system, Utf8CP unitName) {return new TestUnit(parentUnit, system, unitName);}
        };
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
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingDerivedUnits)
    {
    TestUnit const* testConstant = UnitRegistry::Get().AddConstant<TestUnit>("NUMBER", "SI", "TestConstant", "ONE", 42);
    ASSERT_NE(nullptr, testConstant);
    UnitCP retrievedConstant = UnitRegistry::Get().LookupUnit("TestConstant");
    EXPECT_EQ(retrievedConstant, testConstant);
    TestUnit const* retrievedTestConstant = dynamic_cast<TestUnit const*>(retrievedConstant);
    EXPECT_NE(nullptr, retrievedTestConstant);

    TestUnit const* smoot = UnitRegistry::Get().AddUnit<TestUnit>("LENGTH", "USCustom", "Smoot", "M", 1.7);
    ASSERT_NE(nullptr, smoot);
    UnitCP retrievedSmoot = UnitRegistry::Get().LookupUnit("Smoot");
    EXPECT_EQ(retrievedSmoot, smoot);
    TestUnit const* retrievedSmootAsTestUnit = dynamic_cast<TestUnit const*>(retrievedSmoot);
    EXPECT_NE(nullptr, retrievedSmootAsTestUnit);
    
    TestUnit const* testUnit = UnitRegistry::Get().AddUnit<TestUnit>("LENGTH", "SI", "TestUnit", "ONE");
    ASSERT_NE(nullptr, testUnit);
    UnitCP retrievedUnit = UnitRegistry::Get().LookupUnit("TestUnit");
    EXPECT_EQ(retrievedUnit, testUnit);
    TestUnit const* retrievedTestUnit = dynamic_cast<TestUnit const*>(retrievedUnit);
    EXPECT_NE(nullptr, retrievedTestUnit);

    TestUnit const* testInverseUnit = UnitRegistry::Get().AddInvertedUnit<TestUnit>("TestUnit", "InverseTestUnit", "SI");
    ASSERT_NE(nullptr, testInverseUnit);
    UnitCP retrievedInverseUnit = UnitRegistry::Get().LookupUnit("InverseTestUnit");
    EXPECT_EQ(retrievedInverseUnit, testInverseUnit);
    TestUnit const* retrievedInverseTestUnit = dynamic_cast<TestUnit const*>(retrievedInverseUnit);
    EXPECT_NE(nullptr, retrievedInverseTestUnit);

    TestUnit const* smootPerSmoot = UnitRegistry::Get().AddUnit<TestUnit>("SLOPE", "USCustom", "SmootPerSmoot", "Smoot*Smoot(-1)");
    ASSERT_NE(nullptr, smootPerSmoot);
    UnitCP retrievedSmootPerSmoot = UnitRegistry::Get().LookupUnit("SmootPerSmoot");
    EXPECT_EQ(retrievedSmootPerSmoot, smootPerSmoot);

    TestUnit const* inverseSmootPerSmoot = UnitRegistry::Get().AddInvertedUnit<TestUnit>("SmootPerSmoot", "InverseSmootPerSmoot", "USCustom");
    ASSERT_NE(nullptr, inverseSmootPerSmoot);
    UnitCP retrievedInverseSmootPerSmoot = UnitRegistry::Get().LookupUnit("InverseSmootPerSmoot");
    EXPECT_EQ(retrievedInverseSmootPerSmoot, inverseSmootPerSmoot);
    TestUnit const* retrievedInverseSmootPerSmootAsTestUnit = dynamic_cast<TestUnit const*>(retrievedInverseSmootPerSmoot);
    EXPECT_NE(nullptr, retrievedInverseSmootPerSmootAsTestUnit);
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
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingDerivedUnitSystems)
    {
    TestUnitSystem const* testSystem = UnitRegistry::Get().AddSystem<TestUnitSystem>("TestSystem");
    ASSERT_NE(nullptr, testSystem);
    UnitSystemCP retrievedSystem = UnitRegistry::Get().LookupUnitSystem("TestSystem");
    ASSERT_EQ(testSystem, retrievedSystem);

    TestUnitSystem const* retrievedTestSystem = dynamic_cast<TestUnitSystem const*>(retrievedSystem);
    EXPECT_NE(nullptr, retrievedTestSystem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingDerivedPhenomenon)
    {
    TestPhenomenon const* testPhenom = UnitRegistry::Get().AddPhenomenon<TestPhenomenon>("TestPhenomenon", "LENGTH*LENGTH");
    ASSERT_NE(nullptr, testPhenom);
    PhenomenonCP retrievedPhenom = UnitRegistry::Get().LookupPhenomenon("TestPhenomenon");
    ASSERT_EQ(testPhenom, retrievedPhenom);

    TestPhenomenon const* retrievedTestPhenom = dynamic_cast<TestPhenomenon const*>(retrievedPhenom);
    EXPECT_NE(nullptr, retrievedTestPhenom);

    bool hasDerivedPhenomenon = UnitRegistry::Get().HasPhenomenon("TestPhenomenon");
    ASSERT_TRUE(hasDerivedPhenomenon);
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
    UnitSystemCP testSystem = UnitRegistry::Get().AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);

    PhenomenonCP testPhenom = UnitRegistry::Get().AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);

    UnitCP testUnit = UnitRegistry::Get().AddUnit<Unit>("TestPhenomenon", "TestSystem", "TestUnit", "M");
    ASSERT_NE(nullptr, testUnit);

    UnitCP testConstant = UnitRegistry::Get().AddConstant<Unit>("NUMBER", "SI", "TestConstant", "ONE", 42);
    ASSERT_NE(nullptr, testConstant);

    UnitCP testInvUnit = UnitRegistry::Get().AddInvertedUnit<Unit>("TestUnit", "TestInvertedUnit", "TestSystem");
    ASSERT_NE(nullptr, testInvUnit);

    auto names = {"TestSystem", "TestPhenomenon", "TestUnit", "TestConstant", "TestInvertedUnit"};
    for(const auto& name : names)
        {
        ASSERT_EQ(nullptr, UnitRegistry::Get().AddUnit("TestPhenomenon","TestSystem", name, "M"));
        }
    for(const auto& name : names)
        {
        ASSERT_EQ(nullptr, UnitRegistry::Get().AddConstant("TestPhenomenon","TestSystem", name, "M", 10.0));
        }
    for(const auto& name : names)
        {
        ASSERT_EQ(nullptr, UnitRegistry::Get().AddInvertedUnit("TestUnit", name, "TestSystem"));
        }
    for(const auto& name : names)
        {
        ASSERT_EQ(nullptr, UnitRegistry::Get().AddSystem(name));
        }
    for(const auto& name : names)
        {
        ASSERT_EQ(nullptr, UnitRegistry::Get().AddPhenomenon(name, "LENGTH"));
        }
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

    TestUnit const* testConstant = UnitRegistry::Get().AddConstant<TestUnit>("NUMBER", "SI", "TestConstant", "ONE", 42);
    ASSERT_NE(nullptr, testConstant);
    UnitCP retrievedConstant = UnitRegistry::Get().LookupUnit("testconstant");
    EXPECT_EQ(retrievedConstant, testConstant);

    TestUnit const* smoot = UnitRegistry::Get().AddUnit<TestUnit>("LENGTH", "USCustom", "Smoot", "M", 1.7);
    ASSERT_NE(nullptr, smoot);
    UnitCP retrievedSmoot = UnitRegistry::Get().LookupUnit("SmOoT");
    EXPECT_EQ(retrievedSmoot, smoot);

    TestUnit const* smootPerSmoot = UnitRegistry::Get().AddUnit<TestUnit>("SLOPE", "USCustom", "SmootPerSmoot", "Smoot*Smoot(-1)");
    ASSERT_NE(nullptr, smootPerSmoot);
    UnitCP retrievedSmootPerSmoot = UnitRegistry::Get().LookupUnit("SmootPerSmoot");
    EXPECT_EQ(retrievedSmootPerSmoot, smootPerSmoot);

    TestUnit const* inverseSmootPerSmoot = UnitRegistry::Get().AddInvertedUnit<TestUnit>("SmootPerSmoot", "InverseSmootPerSmoot", "USCustom");
    ASSERT_NE(nullptr, inverseSmootPerSmoot);
    UnitCP retrievedInverseSmootPerSmoot = UnitRegistry::Get().LookupUnit("INVERSESMOOTPERSMOOT");
    EXPECT_EQ(retrievedInverseSmootPerSmoot, inverseSmootPerSmoot);
    }


// TODO: find a way to test compile time assertions.. Interesting talk on it, https://www.youtube.com/watch?time_continue=2105&v=zxDzMjfsgjg. Maybe try to use this style? 
#if 0
//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingBadUnits)
    {
    struct NotAUnit
        {
        private:
            NotAUnit(UnitSystemCR unitSystem, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant) { }

        protected:
        // Want to verify that just having a proper API does not allow the template to work.
        static NotAUnit* _Create(UnitSystemCR sysName, PhenomenonCR phenomenon, Utf8CP unitName, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant)
            {return new NotAUnit(sysName, phenomenon, unitName, id, definition, baseSymbol, factor, offset, isConstant);}
        };

    UnitRegistry::Get().AddUnit<NotAUnit>("NUMBER", "SI", "TestUnit", "TEST");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingBadUnitSystem)
    {
    struct BadUnitSystem
        {
        private:
            BadUnitSystem(Utf8CP name) {}
        protected:
            static BadUnitSystem* _Create(Utf8CP name) {return new BadUnitSystem(name);}
        };

    UnitRegistry::Get().AddSystem<BadUnitSystem>("TestUnitSystem");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingBadPhenomenon)
    {
    struct BadPhenomenon
        {
        private:
            BadPhenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) {}
        protected:
            static BadPhenomenon* _Create(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) { return new BadPhenomenon(name, definition, baseSymbol, id); }
        };

    UnitRegistry::Get().AddPhenomenon<BadPhenomenon>("TestPhenomenon");
    }
#endif

END_UNITS_UNITTESTS_NAMESPACE
