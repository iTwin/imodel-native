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
    protected:
        static TestUnitSystem* _Create(Utf8CP name) {return new TestUnitSystem(name);}
    };

    struct TestPhenomenon : Phenomenon
    {
    public:
        TestPhenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) :
            Phenomenon(name, definition, baseSymbol, id) {}
    };

    struct TestUnit : Unit
    {
    friend struct UnitRegistry;
    private:
        TestUnit(UnitSystemCR unitSystem, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant) :
            Unit(unitSystem, phenomenon, name, id, definition, baseSymbol, factor, offset, isConstant) { }

        TestUnit(UnitCR parentUnit, Utf8CP name, uint32_t id)
        : TestUnit(*(parentUnit.GetUnitSystem()), *(parentUnit.GetPhenomenon()), name, id, parentUnit.GetDefinition(), ' ', 0, 0, false)
        { }

    protected:
        static TestUnit* _Create(UnitSystemCR sysName, PhenomenonCR phenomenon, Utf8CP unitName, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant)
        {return new TestUnit(sysName, phenomenon, unitName, id, definition, baseSymbol, factor, offset, isConstant);}

        static TestUnit* _Create(UnitCR parentUnit, Utf8CP unitName, uint32_t id) {return new TestUnit(parentUnit, unitName, id);}
    };
};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddAndRetrieveConstant)
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
TEST_F(UnitRegistryTests, AllUnitsNeededForFirstReleaseExist)
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
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("LENGTH"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("MASS"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("TIME"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("TEMPERATURE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("TEMPERATURE_CHANGE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("CURRENT"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("MOLE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("LUMINOSITY"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("ANGLE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("SOLIDANGLE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("FINANCE"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("CAPITA"));
    EXPECT_TRUE(UnitRegistry::Instance().HasPhenomenon("ONE"));
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

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingDerivedUnits)
    {
    TestUnit const* testConstant = UnitRegistry::Instance().AddConstant<TestUnit>("ONE", "TestConstant", "", 0);
    ASSERT_NE(nullptr, testConstant);
    UnitCP retrievedConstant = UnitRegistry::Instance().LookupUnit("TestConstant");
    EXPECT_EQ(retrievedConstant, testConstant);
    TestUnit const* retrievedTestConstant = dynamic_cast<TestUnit const*>(retrievedConstant);
    EXPECT_NE(nullptr, retrievedTestConstant);

    TestUnit const* testUnit = UnitRegistry::Instance().AddUnit<TestUnit>("ONE", "SI", "TestUnit", "TEST");
    ASSERT_NE(nullptr, testUnit);
    UnitCP retrievedUnit = UnitRegistry::Instance().LookupUnit("TestUnit");
    EXPECT_EQ(retrievedUnit, testUnit);
    TestUnit const* retrievedTestUnit = dynamic_cast<TestUnit const*>(retrievedUnit);
    EXPECT_NE(nullptr, retrievedTestUnit);

    TestUnit const* testInverseUnit = UnitRegistry::Instance().AddInvertingUnit<TestUnit>("TestUnit", "InverseTestUnit");
    ASSERT_NE(nullptr, testInverseUnit);
    UnitCP retrievedInverseUnit = UnitRegistry::Instance().LookupUnit("InverseTestUnit");
    EXPECT_EQ(retrievedInverseUnit, testInverseUnit);
    TestUnit const* retrievedInverseTestUnit = dynamic_cast<TestUnit const*>(retrievedInverseUnit);
    EXPECT_NE(nullptr, retrievedInverseTestUnit);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingDerivedUnitSystems)
    {
    TestUnitSystem const* testSystem = UnitRegistry::Instance().AddSystem<TestUnitSystem>("TestSystem");
    ASSERT_NE(nullptr, testSystem);
    UnitSystemCP retrievedSystem = UnitRegistry::Instance().LookupUnitSystem("TestSystem");
    ASSERT_EQ(testSystem, retrievedSystem);

    TestUnitSystem const* retrievedTestSystem = dynamic_cast<TestUnitSystem const*>(retrievedSystem);
    EXPECT_NE(nullptr, retrievedTestSystem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemovingAUnitSystem)
    {
    UnitSystemCP testSystem = UnitRegistry::Instance().AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);

    UnitSystemCP removedSystem = UnitRegistry::Instance().RemoveSystem("TestSystem");
    ASSERT_EQ(testSystem, removedSystem);
    ASSERT_FALSE(UnitRegistry::Instance().HasSystem("TestSystem"));

    EXPECT_STREQ("TestSystem", removedSystem->GetName().c_str());
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

    UnitRegistry::Instance().AddUnit<NotAUnit>("ONE", "SI", "TestUnit", "TEST");
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

    UnitRegistry::Instance().AddSystem<BadUnitSystem>("TestUnitSystem");
    }
#endif

END_UNITS_UNITTESTS_NAMESPACE
