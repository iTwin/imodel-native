/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/FunctionalDomain_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/FunctionalDomain.h>

#define FUNCTEST_DOMAIN_NAME            "FunctionalTest"
#define FUNCTEST_CLASS_TestBreakdown    "TestBreakdown"
#define FUNCTEST_CLASS_TestComponent    "TestComponent"

DEFINE_POINTER_SUFFIX_TYPEDEFS(TestBreakdown)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TestComponent)

DEFINE_REF_COUNTED_PTR(TestBreakdown)
DEFINE_REF_COUNTED_PTR(TestComponent)

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct FunctionalTestDomain : DgnDomain
{
private:
    DOMAIN_DECLARE_MEMBERS(FunctionalTestDomain, )
    FunctionalTestDomain();

public:
    static Dgn::DgnDbStatus ImportSchema(DgnDbR);
};

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct TestBreakdown : FunctionalBreakdownElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNCTEST_CLASS_TestBreakdown, FunctionalBreakdownElement) 
    friend struct TestBreakdownHandler;

protected:
    TestBreakdown(CreateParams const& params) : T_Super(params) {}

public:
    static TestBreakdownPtr Create(FunctionalModelR, Utf8CP);
    Utf8String GetStringProp1() const {return GetPropertyValueString("StringProp1");}
};

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct TestBreakdownHandler : dgn_ElementHandler::Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(FUNCTEST_CLASS_TestBreakdown, TestBreakdown, TestBreakdownHandler, dgn_ElementHandler::Element, )
};

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct TestComponent : FunctionalComponentElement
{
    DGNELEMENT_DECLARE_MEMBERS(FUNCTEST_CLASS_TestComponent, FunctionalComponentElement) 
    friend struct TestComponentHandler;

protected:
    TestComponent(CreateParams const& params) : T_Super(params) {}

public:
    static TestComponentPtr Create(FunctionalModelR, double);
    double GetDoubleProp1() const {return GetPropertyValueDouble("DoubleProp1");}
};

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct TestComponentHandler : dgn_ElementHandler::Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(FUNCTEST_CLASS_TestComponent, TestComponent, TestComponentHandler, dgn_ElementHandler::Element, )
};

DOMAIN_DEFINE_MEMBERS(FunctionalTestDomain)
HANDLER_DEFINE_MEMBERS(TestBreakdownHandler)
HANDLER_DEFINE_MEMBERS(TestComponentHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
FunctionalTestDomain::FunctionalTestDomain() : DgnDomain(FUNCTEST_DOMAIN_NAME, "Functional Test Schema", 1)
    {
    RegisterHandler(TestBreakdownHandler::GetHandler());
    RegisterHandler(TestComponentHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TestBreakdownPtr TestBreakdown::Create(FunctionalModelR model, Utf8CP stringProp1)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Schemas().GetECClassId(FUNCTEST_DOMAIN_NAME, FUNCTEST_CLASS_TestBreakdown);
    if (!classId.IsValid())
        return nullptr;

    TestBreakdownPtr testBreakdown = new TestBreakdown(CreateParams(db, model.GetModelId(), classId));
    if (DgnDbStatus::Success != testBreakdown->SetProperty("StringProp1", ECN::ECValue(stringProp1)))
        return nullptr;

    return testBreakdown;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TestComponentPtr TestComponent::Create(FunctionalModelR model, double doubleProp1)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Schemas().GetECClassId(FUNCTEST_DOMAIN_NAME, FUNCTEST_CLASS_TestComponent);
    if (!classId.IsValid())
        return nullptr;

    TestComponentPtr testComponent = new TestComponent(CreateParams(db, model.GetModelId(), classId));
    if (DgnDbStatus::Success != testComponent->SetProperty("DoubleProp1", ECN::ECValue(doubleProp1)))
        return nullptr;

    return testComponent;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalTestDomain::ImportSchema(DgnDbR db)
    {
    BeFileName testSchemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    testSchemaFile.AppendToPath(L"ECSchemas");
    testSchemaFile.AppendSeparator();
    testSchemaFile.AppendToPath(L"FunctionalTest.01.00.ecschema.xml");

    DgnDomainR domain = GetDomain();
    return domain.ImportSchema(db, testSchemaFile);
    }

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct FunctionalDomainTests : public DgnDbTestFixture
{
};

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TEST_F(FunctionalDomainTests, FunctionalElementCRUD)
    {
    SetupSeedProject();
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain());
    DgnDomains::RegisterDomain(FunctionalTestDomain::GetDomain());

    DgnDbStatus importSchemaStatus = FunctionalDomain::ImportSchema(*m_db);
    ASSERT_EQ(DgnDbStatus::Success, importSchemaStatus);

    importSchemaStatus = FunctionalTestDomain::ImportSchema(*m_db);
    ASSERT_EQ(DgnDbStatus::Success, importSchemaStatus);

    FunctionalModelPtr model = FunctionalModel::Create(*m_db->Elements().GetRootSubject());
    ASSERT_TRUE(model.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, model->Insert());

    TestBreakdownPtr breakdownA = TestBreakdown::Create(*model, "A");
    TestBreakdownPtr breakdownB = TestBreakdown::Create(*model, "B");
    ASSERT_TRUE(breakdownA.IsValid());
    ASSERT_TRUE(breakdownB.IsValid());
    ASSERT_TRUE(breakdownA->Insert().IsValid());
    ASSERT_TRUE(breakdownB->Insert().IsValid());
    ASSERT_STREQ("A", breakdownA->GetStringProp1().c_str());
    ASSERT_STREQ("B", breakdownB->GetStringProp1().c_str());

    TestComponentPtr component1 = TestComponent::Create(*model, 1.0);
    TestComponentPtr component2 = TestComponent::Create(*model, 2.0);
    TestComponentPtr component3 = TestComponent::Create(*model, 3.0);
    ASSERT_TRUE(component1.IsValid());
    ASSERT_TRUE(component2.IsValid());
    ASSERT_TRUE(component3.IsValid());
    ASSERT_TRUE(component1->Insert().IsValid());
    ASSERT_TRUE(component2->Insert().IsValid());
    ASSERT_TRUE(component3->Insert().IsValid());
    ASSERT_EQ(1.0, component1->GetDoubleProp1());
    ASSERT_EQ(2.0, component2->GetDoubleProp1());
    ASSERT_EQ(3.0, component3->GetDoubleProp1());
    }
