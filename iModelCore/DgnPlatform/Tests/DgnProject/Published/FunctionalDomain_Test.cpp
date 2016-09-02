/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/FunctionalDomain_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/FunctionalDomain.h>

#define FUNCTEST_DOMAIN_NAME                "FunctionalTest"
#define FUNCTEST_CLASS_TestBreakdown        "TestBreakdown"
#define FUNCTEST_CLASS_TestComponent        "TestComponent"
#define FUNCTEST_CLASS_TestFunctionalType   "TestFunctionalType"

DEFINE_POINTER_SUFFIX_TYPEDEFS(TestBreakdown)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TestComponent)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TestFunctionalType)

DEFINE_REF_COUNTED_PTR(TestBreakdown)
DEFINE_REF_COUNTED_PTR(TestComponent)
DEFINE_REF_COUNTED_PTR(TestFunctionalType)

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
struct TestBreakdownHandler : Dgn::func_ElementHandler::FunctionalBreakdownElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(FUNCTEST_CLASS_TestBreakdown, TestBreakdown, TestBreakdownHandler, Dgn::func_ElementHandler::FunctionalBreakdownElementHandler, )
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
struct TestComponentHandler : Dgn::func_ElementHandler::FunctionalComponentElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(FUNCTEST_CLASS_TestComponent, TestComponent, TestComponentHandler, Dgn::func_ElementHandler::FunctionalComponentElementHandler, )
};

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    08/2016
//=======================================================================================
struct TestFunctionalType : Dgn::FunctionalType
{
    DGNELEMENT_DECLARE_MEMBERS(FUNCTEST_CLASS_TestFunctionalType, Dgn::FunctionalType)
    friend struct TestFunctionalTypeHandler;

protected:
    explicit TestFunctionalType(CreateParams const& params) : T_Super(params) {}

public:
    static RefCountedPtr<TestFunctionalType> Create(Dgn::DgnDbR);

    Utf8String GetStringProperty() const {return GetPropertyValueString("StringProperty");}
    void SetStringProperty(Utf8CP s) {SetPropertyValue("StringProperty", s);}

    int32_t GetIntProperty() const {return GetPropertyValueInt32("IntProperty");}
    void SetIntProperty(int32_t i) {SetPropertyValue("IntProperty", i);}
};

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    08/16
//=======================================================================================
struct TestFunctionalTypeHandler : Dgn::func_ElementHandler::FunctionalTypeHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(FUNCTEST_CLASS_TestFunctionalType, TestFunctionalType, TestFunctionalTypeHandler, Dgn::func_ElementHandler::FunctionalTypeHandler, )
};

DOMAIN_DEFINE_MEMBERS(FunctionalTestDomain)
HANDLER_DEFINE_MEMBERS(TestBreakdownHandler)
HANDLER_DEFINE_MEMBERS(TestComponentHandler)
HANDLER_DEFINE_MEMBERS(TestFunctionalTypeHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
FunctionalTestDomain::FunctionalTestDomain() : DgnDomain(FUNCTEST_DOMAIN_NAME, "Functional Test Domain", 1)
    {
    RegisterHandler(TestBreakdownHandler::GetHandler());
    RegisterHandler(TestComponentHandler::GetHandler());
    RegisterHandler(TestFunctionalTypeHandler::GetHandler());
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
    if (DgnDbStatus::Success != testBreakdown->SetPropertyValue("StringProp1", ECN::ECValue(stringProp1)))
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
    if (DgnDbStatus::Success != testComponent->SetPropertyValue("DoubleProp1", ECN::ECValue(doubleProp1)))
        return nullptr;

    return testComponent;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TestFunctionalTypePtr TestFunctionalType::Create(DgnDbR db)
    {
    DgnClassId classId = db.Domains().GetClassId(TestFunctionalTypeHandler::GetHandler());
    return new TestFunctionalType(CreateParams(db, DgnModel::DictionaryId(), classId));
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
    void SetupFunctionalTestDomain();
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
void FunctionalDomainTests::SetupFunctionalTestDomain()
    {
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain());
    DgnDomains::RegisterDomain(FunctionalTestDomain::GetDomain());

    DgnDbStatus importSchemaStatus = FunctionalDomain::ImportSchema(*m_db);
    ASSERT_EQ(DgnDbStatus::Success, importSchemaStatus);

    importSchemaStatus = FunctionalTestDomain::ImportSchema(*m_db);
    ASSERT_EQ(DgnDbStatus::Success, importSchemaStatus);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/2016
//---------------------------------------------------------------------------------------
TEST_F(FunctionalDomainTests, FunctionalDomainCRUD)
    {
    SetupSeedProject();
    SetupFunctionalTestDomain();

    DgnElementId functionalTypeId[3];
    DgnElementId componentId;

    // insert some sample FunctionalTypes
    for (int32_t i=0; i<_countof(functionalTypeId); i++)
        {
        TestFunctionalTypePtr funcType = TestFunctionalType::Create(*m_db);
        ASSERT_TRUE(funcType.IsValid());
        funcType->SetUserLabel(Utf8PrintfString("FunctionalType%d", i).c_str());
        funcType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        funcType->SetIntProperty(i);
        ASSERT_TRUE(funcType->Insert().IsValid());
        functionalTypeId[i] = funcType->GetElementId();
        }

    // flush cache to make sure FunctionalTypes were inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(functionalTypeId); i++)
            {
            TestFunctionalTypePtr funcType = m_db->Elements().GetForEdit<TestFunctionalType>(functionalTypeId[i]);
            ASSERT_TRUE(funcType.IsValid());
            ASSERT_STREQ(funcType->GetStringProperty().c_str(), Utf8PrintfString("String%d", i).c_str());
            ASSERT_EQ(funcType->GetIntProperty(), i);

            funcType->SetStringProperty(Utf8PrintfString("Updated%d", i).c_str());
            funcType->SetIntProperty(i+100);
            ASSERT_TRUE(funcType->Update().IsValid());
            }
        }

    // flush cache to make sure FunctionalTypes were updated properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(functionalTypeId); i++)
            {
            TestFunctionalTypeCPtr funcType = m_db->Elements().Get<TestFunctionalType>(functionalTypeId[i]);
            ASSERT_TRUE(funcType.IsValid());
            ASSERT_STREQ(funcType->GetStringProperty().c_str(), Utf8PrintfString("Updated%d", i).c_str());
            ASSERT_EQ(funcType->GetIntProperty(), i+100);
            }
        }

    FunctionalModelPtr model = FunctionalModel::Create(*m_db->Elements().GetRootSubject());
    ASSERT_TRUE(model.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, model->Insert());

    // create a FunctionalComponent and set its FunctionalType
        {
        TestComponentPtr component = TestComponent::Create(*model, 1.0);
        ASSERT_TRUE(component.IsValid());
        ASSERT_FALSE(component->GetFunctionalTypeId().IsValid());
        ASSERT_FALSE(component->GetFunctionalType().IsValid());
        component->SetFunctionalType(functionalTypeId[0]);
        ASSERT_TRUE(component->GetFunctionalTypeId().IsValid());
        ASSERT_TRUE(component->GetFunctionalType().IsValid());
        ASSERT_TRUE(component->Insert().IsValid());
        componentId = component->GetElementId();
        }

    // flush cache to make sure the FunctionalComponent's FunctionalType was inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        TestComponentPtr component = m_db->Elements().GetForEdit<TestComponent>(componentId);
        ASSERT_TRUE(component.IsValid());
        ASSERT_TRUE(component->GetFunctionalType().IsValid());
        ASSERT_EQ(component->GetFunctionalTypeId().GetValue(), functionalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, component->SetFunctionalType(functionalTypeId[1]));
        ASSERT_TRUE(component->Update().IsValid());
        }

    // flush cache to make sure the FunctionalComponent's FunctionalType was updated properly
        {
        m_db->Memory().PurgeUntil(0);
        TestComponentPtr component = m_db->Elements().GetForEdit<TestComponent>(componentId);
        ASSERT_TRUE(component.IsValid());
        ASSERT_TRUE(component->GetFunctionalType().IsValid());
        ASSERT_EQ(component->GetFunctionalTypeId().GetValue(), functionalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, component->SetFunctionalType(DgnElementId()));
        ASSERT_TRUE(component->Update().IsValid());
        }

    // flush cache to make sure the FunctionalComponent's FunctionalType was cleared properly
        {
        m_db->Memory().PurgeUntil(0);
        TestComponentCPtr component = m_db->Elements().Get<TestComponent>(componentId);
        ASSERT_TRUE(component.IsValid());
        ASSERT_FALSE(component->GetFunctionalType().IsValid());
        ASSERT_FALSE(component->GetFunctionalTypeId().IsValid());
        }

    // test CRUD for FunctionalBreakdownElements and FunctionalComponentElements
    DgnElementId breakdownId[2];
    DgnElementId testComponentId[3];

    for (int i=0; i<_countof(breakdownId); i++)
        {
        TestBreakdownPtr breakdown = TestBreakdown::Create(*model, Utf8PrintfString("Breakdown%d", i).c_str());
        ASSERT_TRUE(breakdown.IsValid());
        ASSERT_TRUE(breakdown->Insert().IsValid());
        breakdownId[i] = breakdown->GetElementId();
        }

    for (int i=0; i<_countof(testComponentId); i++)
        {
        TestComponentPtr component = TestComponent::Create(*model, i);
        ASSERT_TRUE(component.IsValid());
        ASSERT_TRUE(component->Insert().IsValid());
        testComponentId[i] = component->GetElementId();
        }

    // flush cache to make sure the TestBreakdowns and TestComponents were inserted properly
    m_db->Memory().PurgeUntil(0);

    for (int i=0; i<_countof(breakdownId); i++)
        {
        TestBreakdownCPtr breakdown = m_db->Elements().Get<TestBreakdown>(breakdownId[i]);
        ASSERT_TRUE(breakdown.IsValid());
        ASSERT_STREQ(breakdown->GetStringProp1().c_str(), Utf8PrintfString("Breakdown%d", i).c_str());
        }

    for (int i=0; i<_countof(testComponentId); i++)
        {
        TestComponentCPtr component = m_db->Elements().Get<TestComponent>(testComponentId[i]);
        ASSERT_TRUE(component.IsValid());
        ASSERT_EQ(component->GetDoubleProp1(), i);
        }
    }
