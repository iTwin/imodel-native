/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Domain_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <BeSQLite/BeSQLite.h>
#include <WebServices/iModelHub/Client/Client.h>
#include "Bentley/BeTest.h"
#include "WebServices/iModelHub/Common.h"
#include <DgnPlatform/DgnElement.h>
#include "WebServices/iModelHub/Client/Briefcase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

#define SCHEMA_VERSION_TEST_SCHEMA_NAME "SchemaVersionTest"
#define TEST_ELEMENT_CLASS_NAME "TestElement"

DEFINE_POINTER_SUFFIX_TYPEDEFS(TestElement)
DEFINE_REF_COUNTED_PTR(TestElement)

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct TestElement : Dgn::PhysicalElement
    {
    friend struct TestElementHandler;

    DGNELEMENT_DECLARE_MEMBERS(TEST_ELEMENT_CLASS_NAME, Dgn::PhysicalElement)
public:
    TestElement(CreateParams const& params) : Dgn::PhysicalElement(params)
    {}

    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(SCHEMA_VERSION_TEST_SCHEMA_NAME, TEST_ELEMENT_CLASS_NAME)); }

    static TestElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId, DgnCode code = DgnCode())
        {
        return new TestElement(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId, Placement3d(), code));
        }
    };

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct TestElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, Dgn::dgn_ElementHandler::Physical, )
    };

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct SchemaVersionTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(SchemaVersionTestDomain, )
private:
    BeFileName m_relativePath;

    WCharCP _GetSchemaRelativePath() const override { return m_relativePath.GetName(); }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    PhysicalModelPtr InsertPhysicalModel(DgnDbR db, Utf8CP partitionName) const
        {
        SubjectCPtr rootSubject = db.Elements().GetRootSubject();
        PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(*rootSubject, partitionName);
        EXPECT_TRUE(partition.IsValid());
        PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
        EXPECT_TRUE(model.IsValid());
        EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
        return model;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    DgnCategoryId InsertSpatialCategory(DgnDbR db, Utf8CP categoryName) const
        {
        DgnSubCategory::Appearance appearance;
        appearance.SetColor(ColorDef::Black());

        SpatialCategory category(db.GetDictionaryModel(), categoryName, DgnCategory::Rank::Application);
        SpatialCategoryCPtr persistentCategory = category.Insert(appearance);
        EXPECT_TRUE(persistentCategory.IsValid());

        return persistentCategory.IsValid()? persistentCategory->GetCategoryId(): DgnCategoryId();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    void _OnSchemaImported(DgnDbR db) const override
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(db, "SchemaVersionTest");
        if (codeSpec.IsValid())
            codeSpec->Insert();

        PhysicalModelPtr model = InsertPhysicalModel(db, "OnSchemaImportedPartition");
        BeAssert(model.IsValid());

        DgnCategoryId categoryId = InsertSpatialCategory(db, "SchemaVersionTestCategory");
        BeAssert(categoryId.IsValid());

        DgnCode code = CreateCode(db, "OnSchemaImportedElement");

        TestElementPtr el = TestElement::Create(*model, categoryId, code);
        TestElementCPtr cEl = db.Elements().Insert<TestElement>(*el);
        BeAssert(cEl.IsValid());
        }

public:
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    SchemaVersionTestDomain() : DgnDomain(SCHEMA_VERSION_TEST_SCHEMA_NAME, "Version Test Domain", 1)
    {
    }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    void ClearHandlers()
        {
        m_handlers.clear();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    void SetVersion(Utf8CP version)
        {
        m_relativePath.SetNameUtf8(Utf8PrintfString("ECSchemas\\" SCHEMA_VERSION_TEST_SCHEMA_NAME ".%s.ecschema.xml", version));
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    static void Register(Utf8CP version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly)
        {
        SchemaVersionTestDomain::GetDomain().SetVersion(version);
        DgnDomains::RegisterDomain(SchemaVersionTestDomain::GetDomain(), isRequired, isReadonly);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    static DgnCode CreateCode(DgnDbR dgndb, Utf8StringCR value)
        {
        return CodeSpec::CreateCode(dgndb, "SchemaVersionTest", value);
        }
    };

DOMAIN_DEFINE_MEMBERS(SchemaVersionTestDomain)
HANDLER_DEFINE_MEMBERS(TestElementHandler)

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct DomainTests: public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_imodelConnection;


    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();

        auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel(*m_client, "BriefcaseTest");

        m_imodelConnection = ConnectToiModel(*m_client, m_imodel);

        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }


    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    virtual void TearDown() override
        {
        SchemaVersionTestDomain::GetDomain().ClearHandlers();
        if (m_imodel.IsValid())
            DeleteiModel(m_projectId, *m_client, *m_imodel);
        m_client = nullptr;
        IntegrationTestsBase::TearDown();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2017
    //---------------------------------------------------------------------------------------
    BriefcasePtr AcquireBriefcase(bool pull = true, bool forceDomainUpgrade = false)
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel, pull, forceDomainUpgrade);
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(DomainTests, AcquireBriefcaseWithoutSync)
    {
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());

    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false)->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(DomainTests, AcquireBriefcaseChangeSetsInServer)
    {
    // Firstly push some changes
    InitializeWithChangeSets(*m_client, *m_imodel, 2);

    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());

    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory())->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(DomainTests, AcquireBriefcaseWithSchemaChanges)
    {
    // First briefcase makes schema changes
    auto briefcase1 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    db1.CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    ASSERT_TRUE(db1.Txns().HasDbSchemaChanges());
    db1.SaveChanges("ChangeSet 1");
    Utf8String changeSet1 = PushPendingChanges(*briefcase1);

    // Other user acquires briefcase with required domain
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());

    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory())->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(DomainTests, ImportSchemasAndMakeChanges)
    {
    // Firstly push some changes
    InitializeWithChangeSets(*m_client, *m_imodel, 2);

    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());
    
    // Acquire briefcase
    auto briefcase1 = AcquireBriefcase(true, true);
    DgnDbR db1 = briefcase1->GetDgnDb();
    EXPECT_TRUE(db1.Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    // Push changes
    db1.SaveChanges("ChangeSet 1");
    Utf8String changeSet1 = PushPendingChanges(*briefcase1);

    // Try to acquire other briefcase
    auto briefcase2 = AcquireBriefcase(true, true);
    DgnDbR db2 = briefcase2->GetDgnDb();
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db2));
    EXPECT_TRUE(db2.Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    db2.SaveChanges("ChangeSet 2");
    Utf8String changeSet2 = PushPendingChanges(*briefcase2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(DomainTests, DownloadStandaloneBriefcase)
    {
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());

    auto result = m_client->DownloadStandaloneBriefcase(*m_imodel, [=](iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = m_pHost->GetOutputDirectory();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        })->GetResult();

    EXPECT_SUCCESS(result);
    EXPECT_TRUE(result.GetValue().DoesPathExist());
    }
