/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../TestFixture/UnitsTestFixture.h"
#include <fstream>
BEGIN_UNITS_UNITTESTS_NAMESPACE

struct UnitRegistryTests : UnitsTestFixture { };

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddAndRetrieveConstant)
    {
    // Add constant
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

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
    delete(registry);
    }


//=======================================================================================
//! UnitRegistryTests
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestAddingNewBasePhenomenonAndUnit)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

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
    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddDummyUnit)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

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
    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemovePhenomenon)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

    PhenomenonCP testPhenom = registry->AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);

    ASSERT_FALSE(registry->RemovePhenomenon("DoesNotExist"));
    ASSERT_TRUE(registry->RemovePhenomenon("TestPhenomenon"));
    ASSERT_FALSE(registry->HasPhenomenon("TestPhenomenon"));
    ASSERT_FALSE(registry->RemovePhenomenon("TestPhenomenon"));

    delete (registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveUnitSystem)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

    UnitSystemCP testSystem = registry->AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);

    ASSERT_FALSE(registry->RemoveSystem("DoesNotExist"));
    ASSERT_TRUE(registry->RemoveSystem("TestSystem"));
    ASSERT_FALSE(registry->HasSystem("TestSystem"));
    ASSERT_FALSE(registry->RemoveSystem("TestSystem"));

    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveUnits)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

    UnitCP testUnit = registry->AddUnit("LENGTH", "SI", "banana", "M");
    ASSERT_NE(nullptr, testUnit);

    ASSERT_FALSE(registry->RemoveUnit("DoesNotExist"));
    ASSERT_TRUE(registry->RemoveUnit("banana"));
    ASSERT_FALSE(registry->HasUnit("banana"));
    ASSERT_FALSE(registry->RemoveUnit("banana"));

    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveInvertedUnits)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

    UnitCP testUnit = registry->AddInvertedUnit("M", "banana", "SI");
    ASSERT_NE(nullptr, testUnit);

    ASSERT_FALSE(registry->RemoveInvertedUnit("DoesNotExist"));
    ASSERT_TRUE(registry->RemoveInvertedUnit("banana"));
    ASSERT_FALSE(registry->HasUnit("banana"));
    ASSERT_FALSE(registry->RemoveInvertedUnit("banana"));

    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, RemoveConstant)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

    UnitCP testUnit = registry->AddConstant("LENGTH", "SI", "banana", "M", 1.0);
    ASSERT_NE(nullptr, testUnit);

    ASSERT_FALSE(registry->RemoveConstant("DoesNotExist"));
    ASSERT_TRUE(registry->RemoveConstant("banana"));
    ASSERT_FALSE(registry->HasUnit("banana"));
    ASSERT_FALSE(registry->RemoveConstant("banana"));

    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, AddDuplicateItems)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

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
    
    delete(registry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitRegistryTests, TestCaseInsensitiveLookup)
    {
    UnitRegistry* registry = new UnitRegistry();
    FillRegistry(registry);

    UnitSystemCP testSystem = registry->AddSystem("TestSystem");
    ASSERT_NE(nullptr, testSystem);
    UnitSystemCP retrievedSystem = registry->LookupUnitSystem("testSYSTEM");
    ASSERT_EQ(retrievedSystem, testSystem);

    PhenomenonCP testPhenom = registry->AddPhenomenon("TestPhenomenon", "LENGTH");
    ASSERT_NE(nullptr, testPhenom);
    PhenomenonCP retrievedPhenomenon = registry->LookupPhenomenon("TESTPHENOMENOn");
    ASSERT_EQ(retrievedPhenomenon, testPhenom);
    delete(registry);
    }



END_UNITS_UNITTESTS_NAMESPACE
