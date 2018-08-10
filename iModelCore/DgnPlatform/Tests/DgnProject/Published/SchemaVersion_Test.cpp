/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/SchemaVersion_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_EC

#define SCHEMA_VERSION_TEST_SCHEMA_NAME "SchemaVersionTest"
#define TEST_ELEMENT_CLASS_NAME "TestElement"
#define ANOTHER_TEST_ELEMENT_CLASS_NAME "AnotherTestElement"

DEFINE_POINTER_SUFFIX_TYPEDEFS(TestElement)
DEFINE_REF_COUNTED_PTR(TestElement)
DEFINE_POINTER_SUFFIX_TYPEDEFS(AnotherTestElement)
DEFINE_REF_COUNTED_PTR(AnotherTestElement)

// Turn this on for debugging.
//#define DUMP_REVISION 1

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DgnCore"))

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
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
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct AnotherTestElement : Dgn::PhysicalElement
    {
    friend struct AnotherTestElementHandler;

    DGNELEMENT_DECLARE_MEMBERS(ANOTHER_TEST_ELEMENT_CLASS_NAME, Dgn::PhysicalElement)
    public:
        AnotherTestElement(CreateParams const& params) : Dgn::PhysicalElement(params)
            {}

        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(SCHEMA_VERSION_TEST_SCHEMA_NAME, ANOTHER_TEST_ELEMENT_CLASS_NAME)); }

        static AnotherTestElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId, DgnCode code = DgnCode())
            {
            return new AnotherTestElement(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId, Placement3d(), code));
            }
    };


//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct TestElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, Dgn::dgn_ElementHandler::Physical, )
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct AnotherTestElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ANOTHER_TEST_ELEMENT_CLASS_NAME, AnotherTestElement, AnotherTestElementHandler, Dgn::dgn_ElementHandler::Physical, )
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct SchemaVersionTestDomain : DgnDomain
{
DOMAIN_DECLARE_MEMBERS(SchemaVersionTestDomain, )
private:
    BeFileName m_relativePath;

    WCharCP _GetSchemaRelativePath() const override { return m_relativePath.GetName(); }
    void _OnSchemaImported(DgnDbR db) const override
        {
        PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(db, "OnSchemaImportedPartition");
        BeAssert(model.IsValid());

        DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "SchemaVersionTestCategory");
        BeAssert(categoryId.IsValid());

        DgnCode code = CreateCode(db, "OnSchemaImportedElement");
            
        TestElementPtr el = TestElement::Create(*model, categoryId, code);
        TestElementCPtr cEl = db.Elements().Insert<TestElement>(*el);
        BeAssert(cEl.IsValid());
        }

public:
    SchemaVersionTestDomain() : DgnDomain(SCHEMA_VERSION_TEST_SCHEMA_NAME, "Version Test Domain", 1)
        {
        }

    void ClearHandlers()
        {
        m_handlers.clear();
        }

    void SetVersion(Utf8CP version)
        {
        m_relativePath.SetNameUtf8(Utf8PrintfString("ECSchemas\\" SCHEMA_VERSION_TEST_SCHEMA_NAME ".%s.ecschema.xml", version));
        }

    static void Register(Utf8CP version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly)
        {
        SchemaVersionTestDomain::GetDomain().SetVersion(version);
        DgnDomains::RegisterDomain(SchemaVersionTestDomain::GetDomain(), isRequired, isReadonly);
        }

    static DgnCode CreateCode(DgnDbR dgndb, Utf8StringCR value)
        {
        return CodeSpec::CreateCode(dgndb, "SchemaVersionTest", value);
        }
};

DOMAIN_DEFINE_MEMBERS(SchemaVersionTestDomain)
HANDLER_DEFINE_MEMBERS(TestElementHandler)
HANDLER_DEFINE_MEMBERS(AnotherTestElementHandler)

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct SchemaVersionTestFixture : public DgnDbTestFixture
{
    WCharCP m_copyTestFileName = L"SchemaVersionTestCopy.ibim";

    static void SetUpTestCase()
        {
        ScopedDgnHost tempHost;
        
        //  Request a root seed file.
        DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(true, true));

        //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
        //  Note that we must put our group seed file in a group-specific sub-directory
        SchemaVersionTestFixture::s_seedFileInfo = rootSeedInfo;
        SchemaVersionTestFixture::s_seedFileInfo.fileName.SetName(L"SchemaVersionTestFixture/seed.bim");

        // Customize for my tests
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, SchemaVersionTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
        ASSERT_TRUE(db.IsValid());

        CodeSpecPtr codeSpec = CodeSpec::Create(*db, "SchemaVersionTest");
        if (codeSpec.IsValid())
            codeSpec->Insert();

        db->SaveChanges();
        }

    void TearDown() override
        {
        SchemaVersionTestDomain::GetDomain().ClearHandlers();
        DgnDbTestFixture::TearDown();
        }

    void BackupTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetFileName());
        CloseDb();
        BeFileName originalFile = fileName;
        BeFileName copyFile(originalFile.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);

        BeFileNameStatus fileStatus = BeFileName::BeCopyFile(originalFile.c_str(), copyFile.c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
        }

    void RestoreTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetFileName());
        CloseDb();
        BeFileName originalFile = fileName;
        BeFileName copyFile(originalFile.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);

        BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
        }

    DgnRevisionPtr CreateRevision()
        {
        DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
        if (!revision.IsValid())
            return nullptr;

        RevisionStatus status = m_db->Revisions().FinishCreateRevision();
        if (RevisionStatus::Success != status)
            {
            BeAssert(false);
            return nullptr;
            }

        return revision;
        }

    void DumpRevision(DgnRevisionCR revision, Utf8CP summary)
        {
#ifdef DUMP_REVISION
        LOG.infov("---------------------------------------------------------");
        if (summary != nullptr)
            LOG.infov(summary);
        revision.Dump(*m_db);
        LOG.infov("---------------------------------------------------------");
#endif
        }

};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, ImportDomainSchemas)
    {
    SetupSeedProject();
    BeFileName fileName = m_db->GetFileName();
    SaveDb();
    CloseDb();

    DbResult result = BE_SQLITE_OK;
    DgnDbStatus status = DgnDbStatus::Success;

    /*
    * Reopen Db after optional registration - BE_SQLITE_OK, schemas are not imported
    */
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::No, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_FALSE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));
    CloseDb();

    /*
    * Reopen Db after required registration with default options - BE_SQLITE_ERROR_SchemaUpgradeRequired, schemas are not imported
    */
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaUpgradeRequired);

    /*
    * Reopen Db after explicitly setting up option to validate - BE_SQLITE_ERROR_SchemaUpgradeRequired
    */
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades)));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaUpgradeRequired);

    /*
     * Reopen Db after disallowing domain schema upgrades - the API should allow this (to allow setting up briefcase ids, merge change sets, etc.)
     */
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck)));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    CloseDb();

    /*
    * Reopen Db after read-write registration with UpgradeDomain flag - schemas are imported now
    */
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_TRUE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_TRUE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));

    // Validate that _OnSchemaImport was called
    DgnCode code = SchemaVersionTestDomain::CreateCode(*m_db, "OnSchemaImportedElement");
    DgnElementId elId = m_db->Elements().QueryElementIdByCode(code);
    EXPECT_TRUE(elId.IsValid());

    SaveDb();
    CloseDb();

    /* 
     * Reopen Db - BE_SQLITE_OK, can write to the Db
     */
    SchemaVersionTestDomain::GetDomain().SetReadonly(DgnDomain::Readonly::No);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_TRUE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_TRUE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));

    TestElementPtr el = TestElement::Create(*GetDefaultPhysicalModel(), GetDefaultCategoryId());
    el->SetPropertyValue("IntegerProperty1", 1);
    TestElementCPtr cEl = m_db->Elements().Insert<TestElement>(*el, &status);
    EXPECT_TRUE(cEl.IsValid());

    elId = cEl->GetElementId();

    el = nullptr;
    cEl = nullptr;
    SaveDb();
    CloseDb();

    /*
    * Reopen Db after read-only registration - no error, but cannot insert or update or delete elements
    */
    SchemaVersionTestDomain::GetDomain().SetReadonly(DgnDomain::Readonly::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    el = TestElement::Create(*GetDefaultPhysicalModel(), GetDefaultCategoryId());
    el->SetPropertyValue("IntegerProperty1", 1);
    cEl = m_db->Elements().Insert<TestElement>(*el, &status);
    EXPECT_FALSE(cEl.IsValid());
    EXPECT_TRUE(status == DgnDbStatus::ReadOnlyDomain);

    el = m_db->Elements().GetForEdit<TestElement>(elId);
    EXPECT_TRUE(el.IsValid());
    el->SetPropertyValue("IntegerProperty1", 2);
    cEl = m_db->Elements().Update<TestElement>(*el, &status);
    EXPECT_FALSE(cEl.IsValid());
    EXPECT_TRUE(status == DgnDbStatus::ReadOnlyDomain);

    status = m_db->Elements().Delete(elId);
    EXPECT_TRUE(status == DgnDbStatus::ReadOnlyDomain);

    el = nullptr;
    cEl = nullptr;
    CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, UpgradeDomainSchemas)
    {
    DbResult result = BE_SQLITE_OK;

    //! @note The ECSchemas supplied by registered DgnDomain-s are validated against the corresponding ones in the DgnDb, and 
    //! an appropriate error status is returned if necessary. See table below for the various ECSchema compatibility errors, 
    //! and @see OpenParams for setting the option to upgrade the ECSchema-s. 
    //! <pre>
    //! Sample schema compatibility validation results for an ECSchema in the BIM with Version 2.2.2 (Read.Write.Minor)
    //! -------------------------------------------------------------------------------------------------
    //! Application   |  Validation result                       | Validation result
    //! Version       |  (Readonly)                              | (ReadWrite)
    //! -------------------------------------------------------------------------------------------------
    //! 2.2.2 (same)  | BE_SQLITE_OK                             | BE_SQLITE_OK
    //! -------------------------------------------------------------------------------------------------
    //! 1.2.2 (older) | BE_SQLITE_ERROR_SchemaTooNew             | BE_SQLITE_ERROR_SchemaTooNew
    //! 2.1.2 (older) | BE_SQLITE_OK                             | BE_SQLITE_ERROR_SchemaTooNew
    //! 2.2.1 (older) | BE_SQLITE_OK                             | BE_SQLITE_OK
    //! -------------------------------------------------------------------------------------------------
    //! 3.2.2 (newer) | BE_SQLITE_ERROR_SchemaTooOld             | BE_SQLITE_ERROR_SchemaTooOld
    //! 2.3.2 (newer) | BE_SQLITE_ERROR_SchemaUpgradeRequired    | BE_SQLITE_ERROR_SchemaUpgradeRequired
    //! 2.2.3 (newer) | BE_SQLITE_OK by default, or BE_SQLITE_ERROR_SchemaUpgradeRecommended if 
    //!               | SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades is passed in
    //! -------------------------------------------------------------------------------------------------
    //! </pre>

    /* 
     * Setup BIM with version 2.2.2 
     */
    SetupSeedProject();
    BeFileName fileName = m_db->GetFileName();
    SaveDb();

    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_TRUE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    SchemaVersionTestDomain::GetDomain().SetReadonly(DgnDomain::Readonly::No);

    SaveDb();
    BackupTestFile();

    /* 
     * Application same as BIM 
     */
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();

    /* 
     * Application older than BIM 
     */
    SchemaVersionTestDomain::GetDomain().SetVersion("01.02.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooNew);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooNew);

    SchemaVersionTestDomain::GetDomain().SetVersion("02.01.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooNew);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.01");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    RestoreTestFile();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();

    /* 
     * Application newer than BIM 
     */
    SchemaVersionTestDomain::GetDomain().SetVersion("03.02.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooOld);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooOld);
    BeTest::SetFailOnAssert(false);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooOld);
    BeTest::SetFailOnAssert(true);

    SchemaVersionTestDomain::GetDomain().SetVersion("02.03.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades)));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaUpgradeRecommended);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaUpgradeRequired);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();

    RestoreTestFile();
    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.03");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades)));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaUpgradeRecommended);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades)));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaUpgradeRecommended);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades)));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, CreateAndMergeRevision)
    {
    DbResult result = BE_SQLITE_OK;
    ECPropertyCP testProperty = nullptr;

    /* Setup baseline */
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    BeFileName fileName = m_db->GetFileName();
    BackupTestFile();

    /* Create revision with schema import */
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::No, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    SchemaStatus schemaStatus = SchemaVersionTestDomain::GetDomain().ImportSchema(*m_db);
    EXPECT_EQ(SchemaStatus::Success, schemaStatus);
    
    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty3");
    EXPECT_TRUE(testProperty != nullptr);
    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty == nullptr);

    SaveDb();
    DgnRevisionPtr revision1 = CreateRevision();
    EXPECT_TRUE(revision1.IsValid());
    EXPECT_TRUE(revision1->ContainsSchemaChanges(*m_db));

    DumpRevision(*revision1, "Revision 1");
    CloseDb();

    /* Create revision with schema upgrade */
    SchemaVersionTestDomain::GetDomain().SetVersion("02.03.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty != nullptr);

    SaveDb();
    DgnRevisionPtr revision2 = CreateRevision();
    EXPECT_TRUE(revision2.IsValid());
    EXPECT_TRUE(revision2->ContainsSchemaChanges(*m_db));

    DumpRevision(*revision2, "Revision 2");
    CloseDb();

    /* Restore baseline */
    RestoreTestFile();
   
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    CloseDb();

    /* Merge revision with schema import and validate */
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(*revision1)));
    EXPECT_TRUE(m_db.IsValid());

    EXPECT_TRUE(m_db->Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty3");
    EXPECT_TRUE(testProperty != nullptr);
    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty == nullptr);

    testProperty = nullptr;
    CloseDb();

    /* Merge revision with schema upgrade and validate */
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(*revision2)));
    EXPECT_TRUE(m_db.IsValid());

    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty3");
    EXPECT_TRUE(testProperty != nullptr);
    testProperty = m_db->Schemas().GetClass(TestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty != nullptr);
    
    CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, IncompatibleUpgrade)
    {
    /* Setup baseline */
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    BeFileName fileName = m_db->GetFileName();
    BackupTestFile();

    /* Create revision with initial schema and instance */
    DbResult result;
    SchemaVersionTestDomain::Register("02.02.03", DgnDomain::Required::No, DgnDomain::Readonly::No);
    SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());
    SchemaVersionTestDomain::GetDomain().RegisterHandler(AnotherTestElementHandler::GetHandler());
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    SchemaStatus schemaStatus = SchemaVersionTestDomain::GetDomain().ImportSchema(*m_db);
    EXPECT_EQ(SchemaStatus::Success, schemaStatus);
    EXPECT_TRUE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));

    TestElementPtr el = TestElement::Create(*GetDefaultPhysicalModel(), GetDefaultCategoryId());
    el->SetPropertyValue("IntegerProperty1", 1);
    el->SetPropertyValue("IntegerProperty2", 2);
    DgnDbStatus status;
    TestElementCPtr cEl = m_db->Elements().Insert<TestElement>(*el, &status);
    EXPECT_TRUE(cEl.IsValid());
    DgnElementId elId = cEl->GetElementId();

    AnotherTestElementPtr elA = AnotherTestElement::Create(*GetDefaultPhysicalModel(), GetDefaultCategoryId());
    elA->SetPropertyValue("IntegerProperty5", 5);
    AnotherTestElementCPtr cElA = m_db->Elements().Insert<AnotherTestElement>(*elA, &status);
    EXPECT_TRUE(cElA.IsValid());
    DgnElementId elIdA = cElA->GetElementId();

    SaveDb();
    DgnRevisionPtr revision1 = CreateRevision();
    DumpRevision(*revision1, "Revision 1");
    el = nullptr;
    cEl = nullptr;
    elA = nullptr;
    cElA = nullptr;
    CloseDb();
    m_db = nullptr;

    /* Upgrade schema with incompatible changes (property and class deleted) */

    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.04");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)));
    EXPECT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, result);
    EXPECT_TRUE(!m_db.IsValid());
    }
