/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Domain_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/DgnPlatformLib.h>

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_SCHEMA_NAME1                               "TestSchema"
#define TMTEST_SCHEMA_NAMEW1                             L"TestSchema"
#define TMTEST_SCHEMA_NAME2                               "Test"
#define TMTEST_SCHEMA_NAMEW2                             L"Test"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_CLASS_NAME1                   "Element"

USING_NAMESPACE_BENTLEY_SQLITE

struct ElementHandler_TestSchema;
struct ElementHandler_DgnPlatformSchema;

//=======================================================================================
//! Domains for DgnPlatformSchema and Test Schema
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
struct Domain_DgnSchema : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS (Domain_DgnSchema, )
    public:
        Domain_DgnSchema();
    };

DOMAIN_DEFINE_MEMBERS (Domain_DgnSchema)

//=======================================================================================
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
struct Domain_TestSchema : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS (Domain_TestSchema, )
    public:
        Domain_TestSchema();
    };

DOMAIN_DEFINE_MEMBERS (Domain_TestSchema)

//=======================================================================================
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
struct Domain2_TestSchema : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS (Domain2_TestSchema, )
    public:
        Domain2_TestSchema();
    };

DOMAIN_DEFINE_MEMBERS (Domain2_TestSchema)

//=======================================================================================
//! Test Elements for DgnPlatform Schema and Test Schema
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElement_TestSchema : Dgn::PhysicalElement
    {
    DEFINE_T_SUPER (Dgn::PhysicalElement)

    friend struct ElementHandler_TestSchema;

    public:
        TestElement_TestSchema(CreateParams const& params) : T_Super(params)
            {}

        static ECN::ECClassCP GetTestElementECClass(DgnDbR db)
            {
            return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME1, TMTEST_TEST_ELEMENT_CLASS_NAME1);
            }
        static RefCountedPtr<TestElement_TestSchema> Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);

    };
typedef RefCountedPtr<TestElement_TestSchema> TestElementPtr;
typedef RefCountedCPtr<TestElement_TestSchema> TestElementCPtr;

//=======================================================================================
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
struct TestElement_DgnPlatformSchema : Dgn::PhysicalElement
    {
    DEFINE_T_SUPER (Dgn::PhysicalElement)

    friend struct ElementHandler_DgnPlatformSchema;

    public:
        TestElement_DgnPlatformSchema(CreateParams const& params) : T_Super(params)
            {}

        static ECN::ECClassCP GetTestElementECClass(DgnDbR db)
            {
            return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME);
            }
        static RefCountedPtr<TestElement_DgnPlatformSchema> Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);

    };
typedef RefCountedPtr<TestElement_DgnPlatformSchema> ElementPtr;
typedef RefCountedCPtr<TestElement_DgnPlatformSchema> ElementCPtr;

//=======================================================================================
//! ElementHandlers for DgnPlatform Schema and Test Schema
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct ElementHandler_TestSchema : dgn_ElementHandler::Element
    {
    ELEMENTHANDLER_DECLARE_MEMBERS ("Element", TestElement_TestSchema, ElementHandler_TestSchema, dgn_ElementHandler::Element, )
    };

HANDLER_DEFINE_MEMBERS (ElementHandler_TestSchema)

/*=================================================================================**//**
* @bsiclass                                                     Maha Nasir      07/15
+===============+===============+===============+===============+===============+======*/
struct ElementHandler_DgnPlatformSchema : dgn_ElementHandler::Element
    {
    ELEMENTHANDLER_DECLARE_MEMBERS ("TestElement", TestElement_DgnPlatformSchema, ElementHandler_DgnPlatformSchema, dgn_ElementHandler::Element, )
    };

HANDLER_DEFINE_MEMBERS (ElementHandler_DgnPlatformSchema)

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct DomainTests : public ::testing::Test
    {
    public:
        ScopedDgnHost m_host;
        DgnDbPtr      m_db;
        DgnModelId    m_defaultModelId;
        DgnCategoryId m_defaultCategoryId;
        BeFileName schemaFile1;

        DomainTests();
        ~DomainTests();
        void CloseDb()
            {
            m_db->CloseDb();
            }

        void Setup_Project(WCharCP projFile, WCharCP testFile, Db::OpenMode mode);

        DgnModelR GetDefaultModel()
            {
            return *m_db->Models().GetModel(m_defaultModelId);
            }
          DgnElementCPtr InsertElement_TestSchema(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
          DgnElementCPtr InsertElement_DgnPlatformSchema(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    };

/*=================================================================================**//**
* @bsimethod                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
DgnElementCPtr DomainTests::InsertElement_TestSchema(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement_TestSchema::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
    }

TestElementPtr TestElement_TestSchema::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    TestElementPtr testElement = new TestElement_TestSchema(CreateParams(db,mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId));
        return testElement;
        
    }

/*=================================================================================**//**
* @bsimethod                                                     Maha Nasir      07/15
+===============+===============+===============+===============+===============+======*/
DgnElementCPtr DomainTests::InsertElement_DgnPlatformSchema(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    ElementPtr el = TestElement_DgnPlatformSchema::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
    }

ElementPtr TestElement_DgnPlatformSchema::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    DgnModelPtr model = db.Models().GetModel(mid);

    ElementPtr testElement = new TestElement_DgnPlatformSchema(CreateParams(db,mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId));
    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
//!DgnPlatform Schema Domain.
* @bsimethod                                                    Maha Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Domain_DgnSchema::Domain_DgnSchema() : DgnDomain(TMTEST_SCHEMA_NAME, "Dgn Platform Domain", 1)
    {
    RegisterHandler(ElementHandler_DgnPlatformSchema::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
//!Test Schema Domain 1.
* @bsimethod                                                    Maha Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Domain_TestSchema:: Domain_TestSchema() : DgnDomain(TMTEST_SCHEMA_NAME1, "Test Schema Domain", 1)
    {
    RegisterHandler(ElementHandler_TestSchema::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
//!Test Schema Domain 2.
* @bsimethod                                                    Maha Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Domain2_TestSchema::Domain2_TestSchema() : DgnDomain(TMTEST_SCHEMA_NAME1, "Test Schema Domain 2", 2)
    {}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Maha.Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
    DomainTests::DomainTests()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Maha.Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DomainTests::~DomainTests()
    {}

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DomainTests::Setup_Project(WCharCP projFile, WCharCP testFile, Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE (m_db.IsValid());
    ASSERT_TRUE (result == BE_SQLITE_OK);

    //Default Model
    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE (defaultModel.IsValid());
    GetDefaultModel().FillModel();

    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* Test for checking Domain properties.
* @bsimethod                                    Maha.Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTests, CheckDomainProperties)
    {
    Setup_Project(L"3dMetricGeneral.idgndb", L"DomainTests.idgndb", Db::OpenMode::ReadWrite);

    //Domains Registeration.
    m_db->Domains().RegisterDomain(Domain_DgnSchema::GetDomain());
    m_db->Domains().RegisterDomain(Domain_TestSchema::GetDomain());

    //DgnPlatform Schema
    BeFileName DgnPlatform_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    DgnPlatform_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    auto status = Domain_DgnSchema::GetDomain().ImportSchema(*m_db, DgnPlatform_schemaFile);
    ASSERT_TRUE (DgnDbStatus::Success == status);

    auto dgnp_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    EXPECT_NE (nullptr, dgnp_schema);

    //Test Schema
    BeFileName Test_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    Test_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW1 L".01.00.ecschema.xml");

    auto status1 = Domain_TestSchema::GetDomain().ImportSchema(*m_db, Test_schemaFile);
    ASSERT_TRUE (DgnDbStatus::Success == status1);

    auto test_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    EXPECT_NE (nullptr, test_schema);

    //Tests for DgnPlatform Domain.
    DgnDomainCP dgnp_domain = m_db->Domains().FindDomain(TMTEST_SCHEMA_NAME);
    EXPECT_TRUE (dgnp_domain != nullptr);

    EXPECT_EQ (TMTEST_SCHEMA_NAME, (Utf8String)dgnp_domain->GetDomainName());

    EXPECT_EQ ("Dgn Platform Domain", (Utf8String)dgnp_domain->GetDomainDescription());

    EXPECT_EQ (1, dgnp_domain->GetVersion());

    //Tests for Test Domain.
    DgnDomainCP test_domain = m_db->Domains().FindDomain(TMTEST_SCHEMA_NAME1);
    EXPECT_TRUE (test_domain != nullptr);

    EXPECT_EQ (TMTEST_SCHEMA_NAME1, (Utf8String)test_domain->GetDomainName());

    EXPECT_EQ ("Test Schema Domain", (Utf8String)test_domain->GetDomainDescription());

    EXPECT_EQ (1, test_domain->GetVersion());
    }

/*---------------------------------------------------------------------------------**//**
* Test to verify Element Insertion for two different Handlers in two different Schemas.
* @bsimethod                                    Maha.Nasir           07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTests, InsertElement)
    {
    Setup_Project(L"3dMetricGeneral.idgndb", L"DomainTests.idgndb", Db::OpenMode::ReadWrite);

    //Domains Registeration.
    m_db->Domains().RegisterDomain(Domain_DgnSchema::GetDomain());
    m_db->Domains().RegisterDomain(Domain_TestSchema::GetDomain());

    //DgnPlatform Schema
    BeFileName DgnPlatform_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    DgnPlatform_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    auto status = Domain_DgnSchema::GetDomain().ImportSchema(*m_db, DgnPlatform_schemaFile);
    ASSERT_TRUE (DgnDbStatus::Success == status);

    auto dgnp_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    EXPECT_NE (nullptr, dgnp_schema);

    //Test Schema
    BeFileName Test_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    Test_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW1 L".01.00.ecschema.xml");

    auto status1 = Domain_TestSchema::GetDomain().ImportSchema(*m_db, Test_schemaFile);
    ASSERT_TRUE (DgnDbStatus::Success == status1);

    auto test_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME1, true);
    EXPECT_NE (nullptr, test_schema);

    //Creates a Model.
    auto seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone("model1");
    model1->Insert();

    EXPECT_TRUE (model1 != nullptr);
    DgnModelId M1id = m_db->Models().QueryModelId("model1");
    EXPECT_TRUE (M1id.IsValid());

    //Inserts an element in DgnPlatform Domain.
    auto keyE1 = InsertElement_TestSchema("Ele", M1id);

    DgnElementId E1id = keyE1->GetElementId();
    EXPECT_TRUE (E1id.IsValid());
    DgnElementCP pE1 = m_db->Elements().FindElement(E1id);
    EXPECT_NE (nullptr, pE1);

    //Tests to verify that the element is inserted correctly with the handler.
    DgnClassId ClassId = DgnClassId(TestElement_DgnPlatformSchema::GetTestElementECClass(*m_db)->GetId());

    DgnDomain::Handler* DgnPlatformSchema_handler = m_db->Domains().FindHandler(ClassId, ClassId);  //Handler id and class id are the same for a handler.
    EXPECT_TRUE (DgnPlatformSchema_handler != nullptr);
    EXPECT_EQ ("TestElement", DgnPlatformSchema_handler->GetClassName());

    DgnDomainCR dp_domain = DgnPlatformSchema_handler->GetDomain();
    Utf8CP DomainName = dp_domain.GetDomainName();
    EXPECT_EQ ("DgnPlatformTest", (Utf8String)DomainName);
    Utf8CP DomainDesc = dp_domain.GetDomainDescription();
    EXPECT_EQ ("Dgn Platform Domain", (Utf8String)DomainDesc);

    //Inserts Element in Test Domain.
    auto keyE2 = InsertElement_DgnPlatformSchema("Element", M1id);

    DgnElementId E2id = keyE2->GetElementId();
    EXPECT_TRUE (E2id.IsValid());
    DgnElementCP pE2 = m_db->Elements().FindElement(E2id);
    EXPECT_NE (nullptr, pE2);

    //Tests to verify that the element is inserted correctly with the handler.
    DgnClassId Class_Id = DgnClassId(TestElement_TestSchema::GetTestElementECClass(*m_db)->GetId());

    DgnDomain::Handler* TestSchema_handler = m_db->Domains().FindHandler(Class_Id, Class_Id);  
    EXPECT_TRUE (TestSchema_handler != nullptr);
    EXPECT_EQ ("Element", TestSchema_handler->GetClassName());

    DgnDomainCR test_domain = TestSchema_handler->GetDomain();
    Utf8CP Domain_Name = test_domain.GetDomainName();
    EXPECT_EQ ("TestSchema", (Utf8String)Domain_Name);
    Utf8CP Domain_Desc = test_domain.GetDomainDescription();
    EXPECT_EQ ("Test Schema Domain", (Utf8String)Domain_Desc);
    }

/*---------------------------------------------------------------------------------**//**
* Tests for non existing domain,handler,element and importing a non existing schema.
* @bsimethod                                    Maha.Nasir            07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTests, NonExisting)
{
    Setup_Project(L"3dMetricGeneral.idgndb", L"DomainTests.idgndb", Db::OpenMode::ReadWrite);

    //Domains Registeration.
    m_db->Domains().RegisterDomain(Domain_DgnSchema::GetDomain());
    m_db->Domains().RegisterDomain(Domain_TestSchema::GetDomain());
    m_db->Domains().RegisterDomain(Domain2_TestSchema::GetDomain());

    //Test to find a non existing Domain
    DgnDomainCP domain = m_db->Domains().FindDomain("abc");
    EXPECT_TRUE (domain == nullptr);

    //Test to find a non existing Handler
    int64_t val=30;
    DgnDomain::Handler* handler = m_db->Domains().FindHandler(DgnClassId(val), DgnClassId(val));
    EXPECT_TRUE (handler == nullptr);

    //Tests to find a non existing Element.
    DgnElementId id;
    DgnElementCP pE = m_db->Elements().FindElement(id);
    EXPECT_TRUE (pE==nullptr);

    //Test for a non existing Schema.
    auto schema = m_db->Schemas().GetECSchema("Testing");
    EXPECT_TRUE (schema == nullptr);

    //Importing a non existing schema
    BeFileName Test_schemaFile1(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    Test_schemaFile1.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW2 L".01.00.ecschema.xml");

    BeTest::SetFailOnAssert(false);
    auto stat = Domain_TestSchema::GetDomain().ImportSchema(*m_db, Test_schemaFile1);
    EXPECT_FALSE (DgnDbStatus::Success == stat);
}

/*---------------------------------------------------------------------------------**//**
* Test to check the behaviour if we register multiple domains in the same schema.
* @bsimethod                                    Maha.Nasir             07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTests, MultipleDomainRegister)
    {
    Setup_Project(L"3dMetricGeneral.idgndb", L"DomainTests.idgndb", Db::OpenMode::ReadWrite);

    //Domains Registeration.
    m_db->Domains().RegisterDomain(Domain_DgnSchema::GetDomain());
    m_db->Domains().RegisterDomain(Domain_TestSchema::GetDomain());  //In case of multiple domains in the same schema, only The domain registered first will be in the Db.
    m_db->Domains().RegisterDomain(Domain2_TestSchema::GetDomain()); 
    m_db->Domains().RegisterDomain(Domain_TestSchema::GetDomain()); //Multiple registeration of Domain_TestSchema should prompt some error. But it does'nt.

    //Test Schema
    BeFileName Test_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    Test_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW1 L".01.00.ecschema.xml");

    auto status = Domain_TestSchema::GetDomain().ImportSchema(*m_db, Test_schemaFile);
    ASSERT_TRUE (DgnDbStatus::Success == status);

    auto test_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME1, true);
    EXPECT_NE (nullptr, test_schema);

    //Tests to check that in case of multiple domains in the same schema, the domain registered first will be in the Db only.
    DgnDomainCP domain = m_db->Domains().FindDomain("TestSchema");
    EXPECT_TRUE (domain != nullptr);

    Utf8String name(domain->GetDomainName());
    EXPECT_TRUE (name.Equals("TestSchema"));

    //These tests shows that the domain registered second is not in the DB.
    Utf8String desc(domain->GetDomainDescription());
    EXPECT_TRUE (!desc.Equals("Test Schema Domain 2"));

    int32_t version=domain->GetVersion();
    EXPECT_NE (2, version);
    }

/*---------------------------------------------------------------------------------**//**
* Test to check the behaviour if we import multiple schemas in the same domain.
* @bsimethod                                    Maha.Nasir             07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTests, MultipleSchemasImport)
    {
    Setup_Project(L"3dMetricGeneral.idgndb", L"DomainTests.idgndb", Db::OpenMode::ReadWrite);

    //Domain Registeration.
    m_db->Domains().RegisterDomain(Domain_DgnSchema::GetDomain());

    //DgnPlatform Schema
    BeFileName DgnPlatform_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    DgnPlatform_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    auto status = Domain_DgnSchema::GetDomain().ImportSchema(*m_db, DgnPlatform_schemaFile);
    EXPECT_TRUE (DgnDbStatus::Success == status);

    //Multiple imports of the same schema. Does'nt gives any error.
    auto Status1 = Domain_DgnSchema::GetDomain().ImportSchema(*m_db, DgnPlatform_schemaFile);
    EXPECT_TRUE (DgnDbStatus::Success == Status1);

    auto dgnp_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    EXPECT_NE (nullptr, dgnp_schema);

    //Multiple Schemas in the same domain can't be imported.
    BeFileName Test_schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    Test_schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW1 L".01.00.ecschema.xml");

    auto Status = Domain_DgnSchema::GetDomain().ImportSchema(*m_db, Test_schemaFile);
    EXPECT_FALSE (DgnDbStatus::Success == Status);

    auto test_schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME1, true);
    EXPECT_EQ (nullptr, test_schema);
    }


