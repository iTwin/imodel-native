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

    public:
        TestUnitLocater() { Populate(); }

        UnitCP LocateUnit(Utf8CP name) override {return LocateUnitP(name);}
        UnitP LocateUnitP(Utf8CP name) override;
        PhenomenonCP LocatePhenomenon(Utf8CP name) override {return LocatePhenomenonP(name);}
        PhenomenonP LocatePhenomenonP(Utf8CP name) override {return 0 == strcmp(m_phenomenon->GetName(), name) ? m_phenomenon : nullptr;}
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
UnitP UnitRegistryTests::TestUnitLocater::LocateUnitP(Utf8CP name)
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

    registry->AddUnitLocater(*locater);

    PhenomenonCP locaterPhen = registry->LookupPhenomenon("TestPhen");
    EXPECT_NE(nullptr, locaterPhen);

    UnitCP locaterUnit = registry->LookupUnit("TestUnit1");
    EXPECT_NE(nullptr, locaterUnit);

    registry->RemoveUnitLocater(*locater);
    EXPECT_EQ(nullptr, registry->LookupUnit("TestUnit1"));
    EXPECT_EQ(nullptr, registry->LookupPhenomenon("TestPhen"));
    }

END_UNITS_UNITTESTS_NAMESPACE
